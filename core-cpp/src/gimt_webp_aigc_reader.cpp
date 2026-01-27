//
// Implementation of WebpAIGCReader
//

#include "gimt/gimt_webp_aigc_reader.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"

#include <vector>

namespace gimt {

// WebP RIFF constants
constexpr uint32_t FOURCC_RIFF = 0x46464952; // 'RIFF' in little-endian
constexpr uint32_t FOURCC_WEBP = 0x50424557; // 'WEBP' in little-endian
constexpr uint32_t FOURCC_XMP  = 0x20504D58; // 'XMP ' in little-endian

// Helper function to read uint32 in little-endian
inline uint32_t readU32LE(BinaryReader* reader) {
  uint8_t buf[4];
  if (reader->readBytes(buf, 4) != 4) return 0;
  return static_cast<uint32_t>(buf[0]) |
         (static_cast<uint32_t>(buf[1]) << 8) |
         (static_cast<uint32_t>(buf[2]) << 16) |
         (static_cast<uint32_t>(buf[3]) << 24);
}

bool WebpAIGCReader::prepare(const std::string &filepath) {
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

bool WebpAIGCReader::readAIGCInfo(gimt::AIGCInfo &info) {
  if (!reader) {
    return false;
  }

  // 每次读取前将流重置到开头，保证重复调用的一致性
  stream.clear();
  stream.seekg(0, std::ios::beg);

  // 验证 RIFF 头 (12 bytes)
  // 格式: 'RIFF' (4 bytes) + File Size (4 bytes) + 'WEBP' (4 bytes)
  uint32_t riffTag = readU32LE(reader.get());
  if (riffTag != FOURCC_RIFF) {
    return false;
  }

  // 读取文件大小 (不需要验证，只是跳过)
  uint32_t fileSize = readU32LE(reader.get());
  (void)fileSize; // 未使用

  uint32_t webpTag = readU32LE(reader.get());
  if (webpTag != FOURCC_WEBP) {
    return false;
  }

  // 循环解析 WebP Chunks
  while (!reader->isEOF()) {
    // 读取 Chunk FourCC (4 bytes, little-endian)
    uint32_t chunkId = readU32LE(reader.get());
    if (chunkId == 0) {
      // 读取失败，可能到达文件末尾
      break;
    }

    // 读取 Chunk Size (4 bytes, little-endian)
    uint32_t chunkSize = readU32LE(reader.get());
    if (chunkSize == 0 && reader->isEOF()) {
      break;
    }

    // 检查是否是 XMP chunk
    if (chunkId == FOURCC_XMP) {
      // 读取 XMP 数据
      std::vector<uint8_t> xmpData(chunkSize);
      if (reader->readBytes(xmpData.data(), chunkSize) != chunkSize) {
        return false;
      }

      // 将 XMP 数据转换为字符串
      std::string xmpStr(reinterpret_cast<const char*>(xmpData.data()), chunkSize);

      // 验证是否包含 XMP 标记
      if (xmpStr.find("<x:xmpmeta") == std::string::npos) {
        // 跳过 padding 并继续查找
        if (chunkSize % 2 != 0) {
          reader->skip(1);
        }
        continue;
      }

      // 从 XMP 中提取 AIGC JSON
      std::string pureJson;
      if (!extractAigcJsonFromXmp(xmpStr, pureJson)) {
        return false;
      }

      // 解析 JSON 到结构体
      AIGCInfo::parseJsonToStruct(pureJson, info);
      return true;
    } else {
      // 不是 XMP chunk，跳过 chunk data
      reader->skip(chunkSize);

      // 处理 Padding: RIFF 要求每个 chunk 的大小必须是偶数
      // 如果 chunkSize 是奇数，需要跳过 1 字节的 padding
      if (chunkSize % 2 != 0) {
        reader->skip(1);
      }
    }
  }

  return false;
}

} // namespace gimt

