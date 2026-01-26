//
// Implementation of PngAIGCReader
//

#include "gimt/gimt_png_aigc_reader.h"
#include "gimt/gimt_xml_utils.h"

#include <vector>
#include <cstring>

namespace gimt {

// PNG 文件签名 (8 bytes)
static const uint8_t PNG_SIGNATURE[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

// PNG Chunk Types (as 4-byte strings)
static const char PNG_CHUNK_IEND[4] = {'I', 'E', 'N', 'D'};
static const char PNG_CHUNK_iTXt[4] = {'i', 'T', 'X', 't'};

// XMP keyword for iTXt chunk
static const char PNG_XMP_KEYWORD[] = "XML:com.adobe.xmp";
static const size_t PNG_XMP_KEYWORD_LEN = 17; // without null terminator

bool PngAIGCReader::prepare(const std::string &filepath) {
  if (stream.is_open()) {
    stream.close();
  }
  stream.clear();

  stream.open(filepath, std::ios::binary);
  if (!stream.is_open()) {
    reader.reset();
    return false;
  }

  reader = std::make_unique<BinaryReader>(stream);
  return true;
}

bool PngAIGCReader::readAIGCInfo(gimt::AIGCInfo &info) {
  if (!reader) {
    return false;
  }

  // 每次读取前将流重置到开头，保证重复调用的一致性
  stream.clear();
  stream.seekg(0, std::ios::beg);

  // 验证 PNG 签名
  uint8_t signature[8];
  if (reader->readBytes(signature, 8) != 8) {
    return false;
  }
  if (!std::equal(PNG_SIGNATURE, PNG_SIGNATURE + 8, signature)) {
    // Not a PNG file
    return false;
  }

  // 循环解析 PNG Chunks
  while (!reader->isEOF()) {
    // 读取 Chunk Length (4 bytes, big-endian)
    uint32_t chunkLength = reader->readU32BE();
    
    // 读取 Chunk Type (4 bytes)
    uint8_t chunkType[4];
    if (reader->readBytes(chunkType, 4) != 4) {
      return false;
    }

    // 检查是否到达 IEND chunk
    if (std::equal(PNG_CHUNK_IEND, PNG_CHUNK_IEND + 4, chunkType)) {
      break;
    }

    // 检查是否是 iTXt chunk
    if (std::equal(PNG_CHUNK_iTXt, PNG_CHUNK_iTXt + 4, chunkType)) {
      // iTXt chunk 格式:
      // - Keyword (null-terminated string)
      // - Compression flag (1 byte)
      // - Compression method (1 byte)
      // - Language tag (null-terminated string)
      // - Translated keyword (null-terminated string)
      // - Text content

      std::vector<uint8_t> chunkData(chunkLength);
      if (reader->readBytes(chunkData.data(), chunkLength) != chunkLength) {
        return false;
      }

      // 跳过 CRC (4 bytes)
      reader->skip(4);

      // 解析 iTXt data
      size_t pos = 0;

      // 1. 读取 Keyword (以 null 结尾)
      if (pos + PNG_XMP_KEYWORD_LEN + 1 > chunkLength) {
        continue;
      }

      // 检查是否是 XMP keyword
      if (std::memcmp(chunkData.data(), PNG_XMP_KEYWORD, PNG_XMP_KEYWORD_LEN) == 0 &&
          chunkData[PNG_XMP_KEYWORD_LEN] == '\0') {
        
        pos = PNG_XMP_KEYWORD_LEN + 1;

        // 2. 读取 Compression flag (1 byte)
        if (pos >= chunkLength) continue;
        uint8_t compressionFlag = chunkData[pos++];

        // 3. 读取 Compression method (1 byte)
        if (pos >= chunkLength) continue;
        uint8_t compressionMethod = chunkData[pos++];

        // 4. 跳过 Language tag (null-terminated)
        while (pos < chunkLength && chunkData[pos] != '\0') {
          pos++;
        }
        if (pos < chunkLength) pos++; // skip null terminator

        // 5. 跳过 Translated keyword (null-terminated)
        while (pos < chunkLength && chunkData[pos] != '\0') {
          pos++;
        }
        if (pos < chunkLength) pos++; // skip null terminator

        // 6. 剩余部分是 XMP content
        if (pos >= chunkLength) {
          continue;
        }

        // 检查是否压缩（通常 XMP 不压缩）
        if (compressionFlag != 0) {
          // 当前实现不支持压缩的 XMP
          continue;
        }

        std::string xmpStr(reinterpret_cast<const char*>(chunkData.data() + pos),
                          chunkLength - pos);

        // 验证是否包含 XMP 标记
        if (xmpStr.find("<x:xmpmeta") == std::string::npos) {
          continue;
        }

        // 从 XMP 中提取 AIGC JSON
        std::string pureJson;
        if (!extractAigcJsonFromXmp(xmpStr, pureJson)) {
          continue;
        }

        // 解析 JSON 到结构体
        AIGCInfo::parseJsonToStruct(pureJson, info);
        return true;
      }
    } else {
      // 不是 iTXt chunk，跳过 chunk data 和 CRC
      reader->skip(chunkLength + 4); // data + CRC
    }
  }

  return false;
}

} // namespace gimt
