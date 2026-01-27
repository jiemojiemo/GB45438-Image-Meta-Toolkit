//
// Implementation of HeifAIGCWriter
//

#include "gimt/gimt_heif_aigc_writer.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_binary_reader.h"
#include "gimt/gimt_binary_writer.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstring>
#include <map>
#include <algorithm>

namespace gimt {

// Helper to read box header from data buffer
bool HeifAIGCWriter::readBoxHeader(size_t offset, BoxHeader& header) const {
  if (offset + 8 > inputData_.size()) {
    return false;
  }

  uint32_t size32 = (static_cast<uint32_t>(inputData_[offset]) << 24) |
                    (static_cast<uint32_t>(inputData_[offset + 1]) << 16) |
                    (static_cast<uint32_t>(inputData_[offset + 2]) << 8) |
                    static_cast<uint32_t>(inputData_[offset + 3]);

  header.type = (static_cast<uint32_t>(inputData_[offset + 4]) << 24) |
                (static_cast<uint32_t>(inputData_[offset + 5]) << 16) |
                (static_cast<uint32_t>(inputData_[offset + 6]) << 8) |
                static_cast<uint32_t>(inputData_[offset + 7]);

  if (size32 == 1) {
    if (offset + 16 > inputData_.size()) {
      return false;
    }
    header.size = 0;
    for (int i = 0; i < 8; i++) {
      header.size = (header.size << 8) | inputData_[offset + 8 + i];
    }
    header.headerSize = 16;
  } else if (size32 == 0) {
    return false;
  } else {
    header.size = size32;
    header.headerSize = 8;
  }

  return header.size >= header.headerSize && header.type != 0;
}

bool HeifAIGCWriter::prepare(const std::string &inputFilepath, const std::string &outputFilepath) {
  prepared_ = false;
  inputData_.clear();
  outputPath_.clear();

  std::ifstream in(inputFilepath, std::ios::binary);
  if (!in.is_open()) {
    return false;
  }

  inputData_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  in.close();

  if (inputData_.size() < 8) {
    return false;
  }

  BoxHeader ftypHeader;
  if (!readBoxHeader(0, ftypHeader)) {
    return false;
  }

  if (ftypHeader.type != HEIF_BOX_FTYP) {
    return false;
  }

  if (ftypHeader.headerSize + 4 > inputData_.size()) {
    return false;
  }

  uint32_t majorBrand = (static_cast<uint32_t>(inputData_[ftypHeader.headerSize]) << 24) |
                        (static_cast<uint32_t>(inputData_[ftypHeader.headerSize + 1]) << 16) |
                        (static_cast<uint32_t>(inputData_[ftypHeader.headerSize + 2]) << 8) |
                        static_cast<uint32_t>(inputData_[ftypHeader.headerSize + 3]);

  bool isValidBrand = (majorBrand == HEIF_BRAND_HEIC || 
                       majorBrand == HEIF_BRAND_MIF1 ||
                       majorBrand == HEIF_BRAND_HEVC ||
                       majorBrand == HEIF_BRAND_HEVX);

  if (!isValidBrand) {
    return false;
  }

  outputPath_ = outputFilepath;
  prepared_ = true;
  return true;
}

// Find the highest existing item ID in the file
uint32_t HeifAIGCWriter::findMaxItemID(size_t metaOffset, size_t metaSize) const {
  uint32_t maxID = 0;
  size_t offset = metaOffset + 8 + 4; // Skip box header + version/flags
  size_t metaEnd = metaOffset + metaSize;

  while (offset + 8 < metaEnd) {
    BoxHeader boxHeader;
    if (!readBoxHeader(offset, boxHeader)) {
      break;
    }

    if (boxHeader.type == HEIF_BOX_IINF) {
      size_t iinfDataOffset = offset + boxHeader.headerSize + 4; // Skip version/flags
      
      if (iinfDataOffset + 2 > metaEnd) break;
      
      uint16_t entryCount = (static_cast<uint16_t>(inputData_[iinfDataOffset]) << 8) |
                            inputData_[iinfDataOffset + 1];
      
      size_t infeOffset = iinfDataOffset + 2;

      for (uint16_t i = 0; i < entryCount && infeOffset + 8 < metaEnd; i++) {
        BoxHeader infeHeader;
        if (!readBoxHeader(infeOffset, infeHeader)) {
          break;
        }

        if (infeHeader.type == HEIF_BOX_INFE) {
          size_t infeDataOffset = infeOffset + infeHeader.headerSize;
          size_t infeDataSize = infeHeader.size - infeHeader.headerSize;
          
          if (infeDataOffset + infeDataSize > inputData_.size() || infeDataSize < 6) {
            infeOffset += infeHeader.size;
            continue;
          }

          uint8_t version = inputData_[infeDataOffset];
          size_t dataOffset = infeDataOffset + 4; // Skip version/flags

          uint32_t itemID = 0;
          if (version == 2) {
            if (dataOffset + 2 > infeDataOffset + infeDataSize) {
              infeOffset += infeHeader.size;
              continue;
            }
            itemID = (static_cast<uint32_t>(inputData_[dataOffset]) << 8) |
                     inputData_[dataOffset + 1];
          } else if (version == 3) {
            if (dataOffset + 4 > infeDataOffset + infeDataSize) {
              infeOffset += infeHeader.size;
              continue;
            }
            itemID = (static_cast<uint32_t>(inputData_[dataOffset]) << 24) |
                     (static_cast<uint32_t>(inputData_[dataOffset + 1]) << 16) |
                     (static_cast<uint32_t>(inputData_[dataOffset + 2]) << 8) |
                     inputData_[dataOffset + 3];
          }

          if (itemID > maxID) {
            maxID = itemID;
          }
        }

        infeOffset += infeHeader.size;
      }

      break;
    }

    offset += boxHeader.size;
  }

  return maxID;
}

// Find XMP item ID in iinf box
uint32_t HeifAIGCWriter::findXmpItemID(size_t metaOffset, size_t metaSize) const {
  size_t offset = metaOffset + 8 + 4; // Skip box header + version/flags
  size_t metaEnd = metaOffset + metaSize;

  while (offset + 8 < metaEnd) {
    BoxHeader boxHeader;
    if (!readBoxHeader(offset, boxHeader)) {
      break;
    }

    if (boxHeader.type == HEIF_BOX_IINF) {
      size_t iinfDataOffset = offset + boxHeader.headerSize + 4; // Skip version/flags
      
      if (iinfDataOffset + 2 > metaEnd) break;
      
      uint16_t entryCount = (static_cast<uint16_t>(inputData_[iinfDataOffset]) << 8) |
                            inputData_[iinfDataOffset + 1];
      
      size_t infeOffset = iinfDataOffset + 2;

      for (uint16_t i = 0; i < entryCount && infeOffset + 8 < metaEnd; i++) {
        BoxHeader infeHeader;
        if (!readBoxHeader(infeOffset, infeHeader)) {
          break;
        }

        if (infeHeader.type == HEIF_BOX_INFE) {
          size_t infeDataOffset = infeOffset + infeHeader.headerSize;
          size_t infeDataSize = infeHeader.size - infeHeader.headerSize;
          
          if (infeDataOffset + infeDataSize > inputData_.size() || infeDataSize < 6) {
            infeOffset += infeHeader.size;
            continue;
          }

          uint8_t version = inputData_[infeDataOffset];
          size_t dataOffset = infeDataOffset + 4; // Skip version/flags

          uint32_t itemID = 0;
          if (version == 2) {
            if (dataOffset + 2 > infeDataOffset + infeDataSize) {
              infeOffset += infeHeader.size;
              continue;
            }
            itemID = (static_cast<uint32_t>(inputData_[dataOffset]) << 8) |
                     inputData_[dataOffset + 1];
            dataOffset += 2;
          } else if (version == 3) {
            if (dataOffset + 4 > infeDataOffset + infeDataSize) {
              infeOffset += infeHeader.size;
              continue;
            }
            itemID = (static_cast<uint32_t>(inputData_[dataOffset]) << 24) |
                     (static_cast<uint32_t>(inputData_[dataOffset + 1]) << 16) |
                     (static_cast<uint32_t>(inputData_[dataOffset + 2]) << 8) |
                     inputData_[dataOffset + 3];
            dataOffset += 4;
          } else {
            infeOffset += infeHeader.size;
            continue;
          }

          dataOffset += 2; // Skip item_protection_index

          if (dataOffset + 4 > infeDataOffset + infeDataSize) {
            infeOffset += infeHeader.size;
            continue;
          }

          uint32_t itemType = (static_cast<uint32_t>(inputData_[dataOffset]) << 24) |
                              (static_cast<uint32_t>(inputData_[dataOffset + 1]) << 16) |
                              (static_cast<uint32_t>(inputData_[dataOffset + 2]) << 8) |
                              inputData_[dataOffset + 3];
          dataOffset += 4;

          if (itemType == HEIF_ITEM_TYPE_MIME) {
            // Skip item_name (null-terminated)
            while (dataOffset < infeDataOffset + infeDataSize && inputData_[dataOffset] != 0) {
              dataOffset++;
            }
            if (dataOffset < infeDataOffset + infeDataSize) dataOffset++;

            // Read content_type (null-terminated)
            std::string contentType;
            while (dataOffset < infeDataOffset + infeDataSize && inputData_[dataOffset] != 0) {
              contentType += static_cast<char>(inputData_[dataOffset]);
              dataOffset++;
            }

            if (contentType == HEIF_XMP_MIME_TYPE) {
              return itemID;
            }
          }
        }

        infeOffset += infeHeader.size;
      }

      return 0;
    }

    offset += boxHeader.size;
  }

  return 0;
}


// Find item location in iloc box
bool HeifAIGCWriter::findItemLocation(size_t metaOffset, size_t metaSize, 
                                       uint32_t itemID, ItemLocation& loc) const {
  size_t offset = metaOffset + 8 + 4; // Skip box header + version/flags
  size_t metaEnd = metaOffset + metaSize;

  while (offset + 8 < metaEnd) {
    BoxHeader boxHeader;
    if (!readBoxHeader(offset, boxHeader)) {
      break;
    }

    if (boxHeader.type == HEIF_BOX_ILOC) {
      size_t ilocDataOffset = offset + boxHeader.headerSize;
      
      if (ilocDataOffset + 4 > metaEnd) break;

      uint8_t version = inputData_[ilocDataOffset];
      ilocDataOffset += 4; // Skip version/flags

      if (ilocDataOffset + 2 > metaEnd) break;

      uint8_t offsetSize = (inputData_[ilocDataOffset] >> 4) & 0x0F;
      uint8_t lengthSize = inputData_[ilocDataOffset] & 0x0F;
      uint8_t baseOffsetSize = (inputData_[ilocDataOffset + 1] >> 4) & 0x0F;
      ilocDataOffset += 2;

      uint32_t itemCount = 0;
      if (version < 2) {
        if (ilocDataOffset + 2 > metaEnd) break;
        itemCount = (static_cast<uint32_t>(inputData_[ilocDataOffset]) << 8) |
                    inputData_[ilocDataOffset + 1];
        ilocDataOffset += 2;
      } else {
        if (ilocDataOffset + 4 > metaEnd) break;
        itemCount = (static_cast<uint32_t>(inputData_[ilocDataOffset]) << 24) |
                    (static_cast<uint32_t>(inputData_[ilocDataOffset + 1]) << 16) |
                    (static_cast<uint32_t>(inputData_[ilocDataOffset + 2]) << 8) |
                    inputData_[ilocDataOffset + 3];
        ilocDataOffset += 4;
      }

      for (uint32_t i = 0; i < itemCount && ilocDataOffset < metaEnd; i++) {
        uint32_t currentItemID = 0;
        
        if (version < 2) {
          if (ilocDataOffset + 2 > metaEnd) break;
          currentItemID = (static_cast<uint32_t>(inputData_[ilocDataOffset]) << 8) |
                          inputData_[ilocDataOffset + 1];
          ilocDataOffset += 2;
        } else {
          if (ilocDataOffset + 4 > metaEnd) break;
          currentItemID = (static_cast<uint32_t>(inputData_[ilocDataOffset]) << 24) |
                          (static_cast<uint32_t>(inputData_[ilocDataOffset + 1]) << 16) |
                          (static_cast<uint32_t>(inputData_[ilocDataOffset + 2]) << 8) |
                          inputData_[ilocDataOffset + 3];
          ilocDataOffset += 4;
        }

        if (version == 1 || version == 2) {
          ilocDataOffset += 2; // Skip construction_method
        }

        ilocDataOffset += 2; // Skip data_reference_index

        uint64_t baseOffset = 0;
        if (baseOffsetSize > 0 && ilocDataOffset + baseOffsetSize <= metaEnd) {
          for (int j = 0; j < baseOffsetSize; j++) {
            baseOffset = (baseOffset << 8) | inputData_[ilocDataOffset + j];
          }
          ilocDataOffset += baseOffsetSize;
        }

        if (ilocDataOffset + 2 > metaEnd) break;
        uint16_t extentCount = (static_cast<uint16_t>(inputData_[ilocDataOffset]) << 8) |
                               inputData_[ilocDataOffset + 1];
        ilocDataOffset += 2;

        if (extentCount > 0 && currentItemID == itemID) {
          uint64_t extentOffset = 0;
          uint64_t extentLength = 0;

          if (offsetSize > 0 && ilocDataOffset + offsetSize <= metaEnd) {
            for (int j = 0; j < offsetSize; j++) {
              extentOffset = (extentOffset << 8) | inputData_[ilocDataOffset + j];
            }
            ilocDataOffset += offsetSize;
          }

          if (lengthSize > 0 && ilocDataOffset + lengthSize <= metaEnd) {
            for (int j = 0; j < lengthSize; j++) {
              extentLength = (extentLength << 8) | inputData_[ilocDataOffset + j];
            }
          }

          loc.itemID = itemID;
          loc.baseOffset = baseOffset;
          loc.extentOffset = extentOffset;
          loc.extentLength = extentLength;
          return true;
        }

        // Skip remaining extents
        for (uint16_t e = 0; e < extentCount; e++) {
          ilocDataOffset += offsetSize + lengthSize;
        }
      }

      return false;
    }

    offset += boxHeader.size;
  }

  return false;
}


// Build infe entry for XMP item
std::vector<uint8_t> HeifAIGCWriter::buildXmpInfeEntry(uint32_t itemID) const {
  std::vector<uint8_t> infe;
  
  // Box size (placeholder)
  BinaryWriter::writeU32BE(infe, 0);
  // Box type 'infe'
  BinaryWriter::writeU32BE(infe, HEIF_BOX_INFE);
  
  // Version 2, flags 0
  infe.push_back(2);
  infe.push_back(0);
  infe.push_back(0);
  infe.push_back(0);
  
  // Item ID (2 bytes for version 2)
  BinaryWriter::writeU16BE(infe, static_cast<uint16_t>(itemID));
  
  // Item protection index (2 bytes)
  BinaryWriter::writeU16BE(infe, 0);
  
  // Item type 'mime'
  BinaryWriter::writeU32BE(infe, HEIF_ITEM_TYPE_MIME);
  
  // Item name (null-terminated, empty)
  infe.push_back(0);
  
  // Content type (null-terminated)
  const char* mimeType = HEIF_XMP_MIME_TYPE;
  infe.insert(infe.end(), mimeType, mimeType + strlen(mimeType));
  infe.push_back(0);
  
  // Update box size
  uint32_t boxSize = static_cast<uint32_t>(infe.size());
  infe[0] = (boxSize >> 24) & 0xFF;
  infe[1] = (boxSize >> 16) & 0xFF;
  infe[2] = (boxSize >> 8) & 0xFF;
  infe[3] = boxSize & 0xFF;
  
  return infe;
}

// Build iloc entry for XMP item
std::vector<uint8_t> HeifAIGCWriter::buildIlocEntry(uint32_t itemID, uint64_t offset, 
                                                      uint64_t length, uint8_t offsetSize, 
                                                      uint8_t lengthSize, uint8_t baseOffsetSize) const {
  std::vector<uint8_t> entry;
  
  // Item ID (2 bytes for version 0)
  BinaryWriter::writeU16BE(entry, static_cast<uint16_t>(itemID));
  
  // Data reference index (2 bytes)
  BinaryWriter::writeU16BE(entry, 0);
  
  // Base offset (baseOffsetSize bytes) - store the offset here for consistency
  for (int i = baseOffsetSize - 1; i >= 0; i--) {
    entry.push_back((offset >> (i * 8)) & 0xFF);
  }
  
  // Extent count (2 bytes)
  BinaryWriter::writeU16BE(entry, 1);
  
  // Extent offset (offsetSize bytes) - set to 0 since we use base_offset
  for (int i = offsetSize - 1; i >= 0; i--) {
    entry.push_back(0);
  }
  
  // Extent length (lengthSize bytes)
  for (int i = lengthSize - 1; i >= 0; i--) {
    entry.push_back((length >> (i * 8)) & 0xFF);
  }
  
  return entry;
}


// Main function to write AIGC info
bool HeifAIGCWriter::writeAIGCInfo(const AIGCInfo &info) {
  if (!prepared_) {
    return false;
  }

  // Build XMP content
  std::string json = info.toJson();
  std::string escapedJson = xmlEscape(json);
  std::string xmpPayload = buildXmpPayload(escapedJson);
  std::vector<uint8_t> xmpData(xmpPayload.begin(), xmpPayload.end());

  // Find meta box
  size_t metaOffset = 0;
  size_t metaSize = 0;
  size_t offset = 0;
  
  while (offset < inputData_.size()) {
    BoxHeader boxHeader;
    if (!readBoxHeader(offset, boxHeader)) {
      break;
    }

    if (boxHeader.type == HEIF_BOX_META) {
      metaOffset = offset;
      metaSize = boxHeader.size;
      break;
    }

    offset += boxHeader.size;
  }

  if (metaOffset == 0) {
    return false;
  }

  // Check if XMP item already exists
  uint32_t existingXmpID = findXmpItemID(metaOffset, metaSize);
  uint32_t maxItemID = findMaxItemID(metaOffset, metaSize);
  uint32_t xmpItemID = existingXmpID > 0 ? existingXmpID : (maxItemID + 1);

  // Parse existing meta box to extract parameters
  size_t metaChildOffset = metaOffset + 8 + 4;
  size_t metaEnd = metaOffset + metaSize;
  
  uint8_t offsetSize = 4;
  uint8_t lengthSize = 4;
  uint8_t baseOffsetSize = 4;
  uint8_t ilocVersion = 0;
  
  // Extract iloc parameters
  while (metaChildOffset + 8 < metaEnd) {
    BoxHeader childHeader;
    if (!readBoxHeader(metaChildOffset, childHeader)) {
      break;
    }

    if (childHeader.type == HEIF_BOX_ILOC) {
      size_t ilocDataOffset = metaChildOffset + childHeader.headerSize;
      ilocVersion = inputData_[ilocDataOffset];
      offsetSize = (inputData_[ilocDataOffset + 4] >> 4) & 0x0F;
      lengthSize = inputData_[ilocDataOffset + 4] & 0x0F;
      baseOffsetSize = (inputData_[ilocDataOffset + 5] >> 4) & 0x0F;
      
      if (offsetSize == 0) offsetSize = 4;
      if (lengthSize == 0) lengthSize = 4;
      if (baseOffsetSize == 0) baseOffsetSize = 4;
      break;
    }

    metaChildOffset += childHeader.size;
  }

  // Calculate where XMP data will be located
  // It will be at the end of the file in a new mdat box
  size_t afterMetaOffset = metaOffset + metaSize;
  size_t afterMetaSize = (afterMetaOffset < inputData_.size()) ? 
                         (inputData_.size() - afterMetaOffset) : 0;

  // Build new iinf and iloc boxes
  std::vector<uint8_t> newIinf;
  std::vector<uint8_t> newIloc;
  
  // Build new meta box
  std::vector<uint8_t> newMeta;
  
  // Meta box header (placeholder)
  BinaryWriter::writeU32BE(newMeta, 0);
  BinaryWriter::writeU32BE(newMeta, HEIF_BOX_META);
  
  // Version/flags
  newMeta.push_back(0);
  newMeta.push_back(0);
  newMeta.push_back(0);
  newMeta.push_back(0);

  // First pass: copy all boxes except iinf and iloc
  metaChildOffset = metaOffset + 8 + 4;
  
  while (metaChildOffset + 8 < metaEnd) {
    BoxHeader childHeader;
    if (!readBoxHeader(metaChildOffset, childHeader)) {
      break;
    }

    if (childHeader.type != HEIF_BOX_IINF && childHeader.type != HEIF_BOX_ILOC) {
      // Copy other boxes as-is
      for (size_t j = 0; j < childHeader.size; j++) {
        newMeta.push_back(inputData_[metaChildOffset + j]);
      }
    }

    metaChildOffset += childHeader.size;
  }

  // Rebuild iinf box
  metaChildOffset = metaOffset + 8 + 4;
  
  while (metaChildOffset + 8 < metaEnd) {
    BoxHeader childHeader;
    if (!readBoxHeader(metaChildOffset, childHeader)) {
      break;
    }

    if (childHeader.type == HEIF_BOX_IINF) {
      size_t iinfDataOffset = metaChildOffset + childHeader.headerSize;
      
      // Copy version/flags
      std::vector<uint8_t> versionFlags(4);
      for (int i = 0; i < 4; i++) {
        versionFlags[i] = inputData_[iinfDataOffset + i];
      }
      
      uint16_t entryCount = (static_cast<uint16_t>(inputData_[iinfDataOffset + 4]) << 8) |
                            inputData_[iinfDataOffset + 5];
      
      // Build new iinf
      BinaryWriter::writeU32BE(newIinf, 0); // Size placeholder
      BinaryWriter::writeU32BE(newIinf, HEIF_BOX_IINF);
      newIinf.insert(newIinf.end(), versionFlags.begin(), versionFlags.end());
      
      uint16_t newEntryCount = existingXmpID > 0 ? entryCount : entryCount + 1;
      BinaryWriter::writeU16BE(newIinf, newEntryCount);
      
      // Copy existing infe entries (except old XMP if replacing)
      size_t infeOffset = iinfDataOffset + 6;
      for (uint16_t i = 0; i < entryCount && infeOffset + 8 < metaEnd; i++) {
        BoxHeader infeHeader;
        if (!readBoxHeader(infeOffset, infeHeader)) {
          break;
        }
        
        bool isOldXmp = false;
        if (existingXmpID > 0 && infeHeader.type == HEIF_BOX_INFE) {
          size_t infeDataOffset = infeOffset + infeHeader.headerSize;
          uint8_t version = inputData_[infeDataOffset];
          size_t idOffset = infeDataOffset + 4;
          
          uint32_t itemID = 0;
          if (version == 2) {
            itemID = (static_cast<uint32_t>(inputData_[idOffset]) << 8) |
                     inputData_[idOffset + 1];
          } else if (version == 3) {
            itemID = (static_cast<uint32_t>(inputData_[idOffset]) << 24) |
                     (static_cast<uint32_t>(inputData_[idOffset + 1]) << 16) |
                     (static_cast<uint32_t>(inputData_[idOffset + 2]) << 8) |
                     inputData_[idOffset + 3];
          }
          
          if (itemID == existingXmpID) {
            isOldXmp = true;
          }
        }
        
        if (!isOldXmp) {
          for (size_t j = 0; j < infeHeader.size; j++) {
            newIinf.push_back(inputData_[infeOffset + j]);
          }
        }
        
        infeOffset += infeHeader.size;
      }
      
      // Add new XMP infe entry
      std::vector<uint8_t> xmpInfe = buildXmpInfeEntry(xmpItemID);
      newIinf.insert(newIinf.end(), xmpInfe.begin(), xmpInfe.end());
      
      // Update iinf size
      uint32_t iinfSize = static_cast<uint32_t>(newIinf.size());
      newIinf[0] = (iinfSize >> 24) & 0xFF;
      newIinf[1] = (iinfSize >> 16) & 0xFF;
      newIinf[2] = (iinfSize >> 8) & 0xFF;
      newIinf[3] = iinfSize & 0xFF;
      
      break;
    }

    metaChildOffset += childHeader.size;
  }

  // Rebuild iloc box - we need to do this AFTER calculating xmpDataOffset
  // So we'll build it in a separate pass after we know the new meta size
  metaChildOffset = metaOffset + 8 + 4;
  
  while (metaChildOffset + 8 < metaEnd) {
    BoxHeader childHeader;
    if (!readBoxHeader(metaChildOffset, childHeader)) {
      break;
    }

    if (childHeader.type == HEIF_BOX_ILOC) {
      size_t ilocDataOffset = metaChildOffset + childHeader.headerSize;
      
      // Copy version/flags
      std::vector<uint8_t> versionFlags(4);
      for (int i = 0; i < 4; i++) {
        versionFlags[i] = inputData_[ilocDataOffset + i];
      }
      
      uint32_t itemCount = 0;
      size_t itemCountOffset = ilocDataOffset + 6;
      if (ilocVersion < 2) {
        itemCount = (static_cast<uint32_t>(inputData_[itemCountOffset]) << 8) |
                    inputData_[itemCountOffset + 1];
      } else {
        itemCount = (static_cast<uint32_t>(inputData_[itemCountOffset]) << 24) |
                    (static_cast<uint32_t>(inputData_[itemCountOffset + 1]) << 16) |
                    (static_cast<uint32_t>(inputData_[itemCountOffset + 2]) << 8) |
                    inputData_[itemCountOffset + 3];
      }
      
      // Build new iloc
      BinaryWriter::writeU32BE(newIloc, 0); // Size placeholder
      BinaryWriter::writeU32BE(newIloc, HEIF_BOX_ILOC);
      newIloc.insert(newIloc.end(), versionFlags.begin(), versionFlags.end());
      newIloc.push_back(inputData_[ilocDataOffset + 4]);
      newIloc.push_back(inputData_[ilocDataOffset + 5]);
      
      uint32_t newItemCount = existingXmpID > 0 ? itemCount : itemCount + 1;
      if (ilocVersion < 2) {
        BinaryWriter::writeU16BE(newIloc, static_cast<uint16_t>(newItemCount));
      } else {
        BinaryWriter::writeU32BE(newIloc, newItemCount);
      }
      
      // Copy existing iloc entries (except old XMP if replacing)
      size_t entryOffset = itemCountOffset + (ilocVersion < 2 ? 2 : 4);
      for (uint32_t i = 0; i < itemCount && entryOffset < metaEnd; i++) {
        uint32_t itemID = 0;
        size_t entryStart = entryOffset;
        
        if (ilocVersion < 2) {
          itemID = (static_cast<uint32_t>(inputData_[entryOffset]) << 8) |
                   inputData_[entryOffset + 1];
          entryOffset += 2;
        } else {
          itemID = (static_cast<uint32_t>(inputData_[entryOffset]) << 24) |
                   (static_cast<uint32_t>(inputData_[entryOffset + 1]) << 16) |
                   (static_cast<uint32_t>(inputData_[entryOffset + 2]) << 8) |
                   inputData_[entryOffset + 3];
          entryOffset += 4;
        }
        
        if (ilocVersion == 1 || ilocVersion == 2) {
          entryOffset += 2; // construction_method
        }
        
        entryOffset += 2; // data_reference_index
        entryOffset += baseOffsetSize; // base_offset
        
        uint16_t extentCount = (static_cast<uint16_t>(inputData_[entryOffset]) << 8) |
                               inputData_[entryOffset + 1];
        entryOffset += 2;
        
        size_t entrySize = entryOffset - entryStart + extentCount * (offsetSize + lengthSize);
        
        if (itemID != existingXmpID) {
          // Copy this entry
          for (size_t j = 0; j < entrySize; j++) {
            newIloc.push_back(inputData_[entryStart + j]);
          }
        }
        
        entryOffset = entryStart + entrySize;
      }
      
      // Note: We'll add the XMP iloc entry after we calculate the correct offset
      
      break;
    }

    metaChildOffset += childHeader.size;
  }

  // Add iinf to meta first (we'll add iloc after calculating the offset)
  newMeta.insert(newMeta.end(), newIinf.begin(), newIinf.end());
  
  // Temporarily add iloc to calculate the final meta size
  size_t metaSizeBeforeIloc = newMeta.size();
  newMeta.insert(newMeta.end(), newIloc.begin(), newIloc.end());
  
  // Calculate what the meta box size will be (we need to add the XMP iloc entry)
  size_t xmpIlocEntrySize = 2 + 2 + baseOffsetSize + 2 + offsetSize + lengthSize; // For version 0
  uint32_t metaBoxSize = static_cast<uint32_t>(newMeta.size() + xmpIlocEntrySize);
  
  // Calculate XMP data offset in the final file
  // Structure: [before_meta][new_meta][after_meta][new_mdat_header][xmp_data]
  uint64_t xmpDataOffset = metaOffset + metaBoxSize + afterMetaSize + 8; // +8 for mdat header
  
  // Now add the XMP iloc entry with the correct offset
  std::vector<uint8_t> xmpIlocEntry = buildIlocEntry(xmpItemID, xmpDataOffset, 
                                                      xmpData.size(), offsetSize, 
                                                      lengthSize, baseOffsetSize);
  newIloc.insert(newIloc.end(), xmpIlocEntry.begin(), xmpIlocEntry.end());
  
  // Update iloc size
  uint32_t ilocSize = static_cast<uint32_t>(newIloc.size());
  newIloc[0] = (ilocSize >> 24) & 0xFF;
  newIloc[1] = (ilocSize >> 16) & 0xFF;
  newIloc[2] = (ilocSize >> 8) & 0xFF;
  newIloc[3] = ilocSize & 0xFF;
  
  // Rebuild meta with the complete iloc
  newMeta.resize(metaSizeBeforeIloc);
  newMeta.insert(newMeta.end(), newIloc.begin(), newIloc.end());
  
  // Update meta box size
  metaBoxSize = static_cast<uint32_t>(newMeta.size());
  newMeta[0] = (metaBoxSize >> 24) & 0xFF;
  newMeta[1] = (metaBoxSize >> 16) & 0xFF;
  newMeta[2] = (metaBoxSize >> 8) & 0xFF;
  newMeta[3] = metaBoxSize & 0xFF;

  // Write output file
  std::ofstream out(outputPath_, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) {
    return false;
  }

  // Write everything before meta
  out.write(reinterpret_cast<const char*>(inputData_.data()), 
            static_cast<std::streamsize>(metaOffset));
  
  // Write new meta box
  out.write(reinterpret_cast<const char*>(newMeta.data()), 
            static_cast<std::streamsize>(newMeta.size()));
  
  // Write everything after meta
  if (afterMetaSize > 0) {
    out.write(reinterpret_cast<const char*>(&inputData_[afterMetaOffset]),
              static_cast<std::streamsize>(afterMetaSize));
  }
  
  // Write XMP data in new mdat box
  std::vector<uint8_t> xmpMdat;
  uint32_t xmpMdatSize = 8 + static_cast<uint32_t>(xmpData.size());
  BinaryWriter::writeU32BE(xmpMdat, xmpMdatSize);
  BinaryWriter::writeU32BE(xmpMdat, HEIF_BOX_MDAT);
  xmpMdat.insert(xmpMdat.end(), xmpData.begin(), xmpData.end());
  
  out.write(reinterpret_cast<const char*>(xmpMdat.data()),
            static_cast<std::streamsize>(xmpMdat.size()));

  return out.good();
}

} // namespace gimt
