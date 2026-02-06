//
// Implementation of HeifAIGCReader
//

#include "gimt/gimt_heif_aigc_reader.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"

#include <vector>
#include <cstring>

namespace gimt {

bool HeifAIGCReader::prepare(const std::string &filepath) {
  if (stream.is_open()) {
    stream.close();
  }
  stream.clear();

  stream.open(filepath, std::ios::binary);
  if (!stream.is_open()) {
    reader.reset();
    prepared_ = false;
    return false;
  }

  reader = std::make_unique<BinaryReader>(stream);
  prepared_ = true;
  return true;
}

// Helper structure to represent a Box header
struct BoxHeader {
  uint64_t size;      // Total size including header
  uint32_t type;      // 4-byte type code
  uint64_t headerSize; // Size of the header itself (8 or 16 bytes)
};

// Helper function to read a box header
static bool readBoxHeader(BinaryReader* reader, BoxHeader& header) {
  if (!reader || reader->isEOF()) {
    return false;
  }

  // Read size (4 bytes, big-endian)
  uint32_t size32 = reader->readU32BE();
  if (size32 == 0) {
    return false;
  }

  // Read type (4 bytes, big-endian)
  header.type = reader->readU32BE();
  if (header.type == 0) {
    return false;
  }

  // Handle extended size (size == 1 means 64-bit size follows)
  if (size32 == 1) {
    // Read 64-bit size
    uint8_t buf[8];
    if (reader->readBytes(buf, 8) != 8) {
      return false;
    }
    header.size = 0;
    for (int i = 0; i < 8; i++) {
      header.size = (header.size << 8) | buf[i];
    }
    header.headerSize = 16; // 4 + 4 + 8
  } else {
    header.size = size32;
    header.headerSize = 8; // 4 + 4
  }

  return true;
}

// Helper function to skip to the end of a box
static void skipBox(BinaryReader* reader, const BoxHeader& header, uint64_t bytesAlreadyRead) {
  if (header.size > header.headerSize + bytesAlreadyRead) {
    uint64_t remaining = header.size - header.headerSize - bytesAlreadyRead;
    reader->skip(remaining);
  }
}

// Helper structure for item location
struct ItemLocation {
  uint32_t itemID;
  uint64_t baseOffset;
  uint64_t extentOffset;
  uint64_t extentLength;
};

bool HeifAIGCReader::readAIGCInfo(gimt::AIGCInfo &info) {
  if (!reader) {
    return false;
  }

  // Reset stream to beginning
  stream.clear();
  stream.seekg(0, std::ios::beg);

  // Step 1: Verify ftyp box (file type)
  BoxHeader ftypHeader;
  if (!readBoxHeader(reader.get(), ftypHeader)) {
    return false;
  }

  if (ftypHeader.type != HEIF_BOX_FTYP) {
    return false;
  }

  // Read major brand (4 bytes)
  uint32_t majorBrand = reader->readU32BE();
  
  // Check if it's a valid HEIF/HEIC brand
  bool isValidBrand = (majorBrand == HEIF_BRAND_HEIC || 
                       majorBrand == HEIF_BRAND_MIF1 ||
                       majorBrand == HEIF_BRAND_HEVC ||
                       majorBrand == HEIF_BRAND_HEVX);
  
  if (!isValidBrand) {
    return false;
  }

  // Skip rest of ftyp box
  skipBox(reader.get(), ftypHeader, 4);

  // Step 2: Find and parse meta box
  uint32_t xmpItemID = 0;
  std::vector<ItemLocation> itemLocations;
  bool foundMeta = false;

  while (!reader->isEOF()) {
    BoxHeader boxHeader;
    if (!readBoxHeader(reader.get(), boxHeader)) {
      break;
    }

    if (boxHeader.type == HEIF_BOX_META) {
      foundMeta = true;
      
      // Meta is a FullBox, skip version and flags (4 bytes)
      reader->skip(4);
      uint64_t metaBytesRead = 4;

      // Parse meta box contents
      uint64_t metaContentSize = boxHeader.size - boxHeader.headerSize - 4;
      std::streamoff metaStartPos = reader->tell();

      while (metaBytesRead < metaContentSize && !reader->isEOF()) {
        BoxHeader metaChildHeader;
        std::streamoff beforeHeader = reader->tell();
        
        if (!readBoxHeader(reader.get(), metaChildHeader)) {
          break;
        }

        if (metaChildHeader.type == HEIF_BOX_IINF) {
          // Item Information Box
          // Skip version and flags (4 bytes)
          reader->skip(4);
          
          // Read entry count (2 or 4 bytes depending on version, we'll read 2 for simplicity)
          uint16_t entryCount = reader->readU16BE();
          
          uint64_t iinfBytesRead = 4 + 2;

          // Parse each infe (item info entry)
          for (uint16_t i = 0; i < entryCount && !reader->isEOF(); i++) {
            BoxHeader infeHeader;
            if (!readBoxHeader(reader.get(), infeHeader)) {
              break;
            }

            if (infeHeader.type == HEIF_BOX_INFE) {
              // Read infe content
              uint64_t infeContentSize = infeHeader.size - infeHeader.headerSize;
              std::vector<uint8_t> infeData(infeContentSize);
              
              if (reader->readBytes(infeData.data(), infeContentSize) != infeContentSize) {
                break;
              }

              // Parse infe structure (version 2 or 3)
              // Offset 0: version (1 byte)
              // Offset 1-3: flags (3 bytes)
              // Offset 4-5 (v2) or 4-7 (v3): item_ID
              
              if (infeContentSize < 6) {
                continue;
              }

              uint8_t version = infeData[0];
              uint32_t itemID = 0;
              size_t offset = 4;

              if (version == 2) {
                itemID = (infeData[4] << 8) | infeData[5];
                offset = 6;
              } else if (version == 3) {
                if (infeContentSize < 8) continue;
                itemID = (infeData[4] << 24) | (infeData[5] << 16) | 
                         (infeData[6] << 8) | infeData[7];
                offset = 8;
              } else {
                continue;
              }

              // Skip item_protection_index (2 bytes)
              offset += 2;

              if (offset + 4 > infeContentSize) {
                continue;
              }

              // Read item_type (4 bytes)
              uint32_t itemType = (infeData[offset] << 24) | (infeData[offset+1] << 16) |
                                  (infeData[offset+2] << 8) | infeData[offset+3];
              offset += 4;

              // Check if this is a mime type item
              if (itemType == HEIF_ITEM_TYPE_MIME) {
                // Read item_name (null-terminated string)
                while (offset < infeContentSize && infeData[offset] != 0) {
                  offset++;
                }
                if (offset < infeContentSize) offset++; // skip null

                // Read content_type (null-terminated string)
                std::string contentType;
                while (offset < infeContentSize && infeData[offset] != 0) {
                  contentType += static_cast<char>(infeData[offset]);
                  offset++;
                }

                // Check if this is XMP (application/rdf+xml)
                if (contentType == HEIF_XMP_MIME_TYPE) {
                  xmpItemID = itemID;
                  // Don't break - we still need to read all entries to properly skip the box
                }
              }

              iinfBytesRead += infeHeader.size;
            }
          }

          // Skip to the end of iinf box
          std::streamoff currentPos = reader->tell();
          std::streamoff iinfEnd = beforeHeader + static_cast<std::streamoff>(metaChildHeader.size);
          if (currentPos < iinfEnd) {
            reader->skip(iinfEnd - currentPos);
          }

        } else if (metaChildHeader.type == HEIF_BOX_ILOC) {
          // Item Location Box
          // Skip version and flags (4 bytes)
          uint8_t versionFlags[4];
          if (reader->readBytes(versionFlags, 4) != 4) {
            break;
          }

          uint8_t version = versionFlags[0];

          // Read offset_size, length_size, base_offset_size, index_size (or reserved)
          uint8_t sizes[2];
          if (reader->readBytes(sizes, 2) != 2) {
            break;
          }

          uint8_t offsetSize = (sizes[0] >> 4) & 0x0F;
          uint8_t lengthSize = sizes[0] & 0x0F;
          uint8_t baseOffsetSize = (sizes[1] >> 4) & 0x0F;
          uint8_t indexSize = (version == 1 || version == 2) ? (sizes[1] & 0x0F) : 0;

          // Read item_count
          uint32_t itemCount = 0;
          if (version < 2) {
            itemCount = reader->readU16BE();
          } else {
            itemCount = reader->readU32BE();
          }

          // Parse each item
          for (uint32_t i = 0; i < itemCount && !reader->isEOF(); i++) {
            ItemLocation loc;
            
            // Read item_ID
            if (version < 2) {
              loc.itemID = reader->readU16BE();
            } else {
              loc.itemID = reader->readU32BE();
            }

            // Skip construction_method (version 1 or 2)
            if (version == 1 || version == 2) {
              reader->skip(2);
            }

            // Read data_reference_index (2 bytes)
            reader->skip(2);

            // Read base_offset
            loc.baseOffset = 0;
            if (baseOffsetSize > 0) {
              uint8_t buf[8] = {0};
              if (reader->readBytes(buf, baseOffsetSize) != baseOffsetSize) {
                break;
              }
              for (int j = 0; j < baseOffsetSize; j++) {
                loc.baseOffset = (loc.baseOffset << 8) | buf[j];
              }
            }

            // Read extent_count
            uint16_t extentCount = reader->readU16BE();

            // Read first extent (we only care about the first one for XMP)
            if (extentCount > 0) {
              // Skip extent_index if present
              if (indexSize > 0 && (version == 1 || version == 2)) {
                reader->skip(indexSize);
              }

              // Read extent_offset
              loc.extentOffset = 0;
              if (offsetSize > 0) {
                uint8_t buf[8] = {0};
                if (reader->readBytes(buf, offsetSize) != offsetSize) {
                  break;
                }
                for (int j = 0; j < offsetSize; j++) {
                  loc.extentOffset = (loc.extentOffset << 8) | buf[j];
                }
              }

              // Read extent_length
              loc.extentLength = 0;
              if (lengthSize > 0) {
                uint8_t buf[8] = {0};
                if (reader->readBytes(buf, lengthSize) != lengthSize) {
                  break;
                }
                for (int j = 0; j < lengthSize; j++) {
                  loc.extentLength = (loc.extentLength << 8) | buf[j];
                }
              }

              itemLocations.push_back(loc);

              // Skip remaining extents
              for (uint16_t e = 1; e < extentCount; e++) {
                if (indexSize > 0 && (version == 1 || version == 2)) {
                  reader->skip(indexSize);
                }
                reader->skip(offsetSize + lengthSize);
              }
            }
          }

          // We've read the iloc box, skip to end
          std::streamoff currentPos = reader->tell();
          std::streamoff boxEnd = beforeHeader + static_cast<std::streamoff>(metaChildHeader.size);
          if (currentPos < boxEnd) {
            reader->skip(boxEnd - currentPos);
          }

        } else {
          // Skip unknown box
          skipBox(reader.get(), metaChildHeader, 0);
        }

        metaBytesRead = static_cast<uint64_t>(static_cast<std::streamoff>(reader->tell()) - metaStartPos);
      }

      break; // Found and processed meta box
    } else {
      // Skip this box
      skipBox(reader.get(), boxHeader, 0);
    }
  }

  if (!foundMeta || xmpItemID == 0) {
    return false;
  }

  // Step 3: Find the location of XMP item
  ItemLocation xmpLocation;
  bool foundLocation = false;
  
  for (const auto& loc : itemLocations) {
    if (loc.itemID == xmpItemID) {
      xmpLocation = loc;
      foundLocation = true;
      break;
    }
  }

  if (!foundLocation || xmpLocation.extentLength == 0) {
    return false;
  }

  // Step 4: Read XMP data from the calculated offset
  uint64_t xmpOffset = xmpLocation.baseOffset + xmpLocation.extentOffset;
  
  stream.clear();
  stream.seekg(xmpOffset, std::ios::beg);
  
  if (stream.fail()) {
    return false;
  }

  std::vector<uint8_t> xmpData(xmpLocation.extentLength);
  if (reader->readBytes(xmpData.data(), xmpLocation.extentLength) != xmpLocation.extentLength) {
    return false;
  }

  // Convert to string
  std::string xmpStr(reinterpret_cast<const char*>(xmpData.data()), xmpLocation.extentLength);

  // Verify it contains XMP metadata (check for either <?xpacket or <x:xmpmeta)
  if (xmpStr.find("<x:xmpmeta") == std::string::npos && xmpStr.find("<?xpacket") == std::string::npos) {
    return false;
  }

  // Step 5: Extract AIGC JSON from XMP
  std::string pureJson;
  if (!extractAigcJsonFromXmp(xmpStr, pureJson)) {
    // Debug: print why extraction failed
    #ifdef DEBUG_HEIF_READER
    std::cerr << "extractAigcJsonFromXmp failed\n";
    std::cerr << "XMP string (first 300 chars): " << xmpStr.substr(0, 300) << "\n";
    #endif
    return false;
  }

  // Step 6: Parse JSON to struct
  AIGCInfo::parseJsonToStruct(pureJson, info);
  return true;
}

} // namespace gimt

