//
// Implementation of WebpAIGCReader
//

#include "gimt/gimt_webp_aigc_reader.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"

#include <vector>

namespace gimt {

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
  uint32_t riffTag = reader->readU32LE();
  if (riffTag != WEBP_FOURCC_RIFF) {
    return false;
  }

  // 读取文件大小 (不需要验证，只是跳过)
  uint32_t fileSize = reader->readU32LE();
  (void)fileSize; // 未使用

  uint32_t webpTag = reader->readU32LE();
  if (webpTag != WEBP_FOURCC_WEBP) {
    return false;
  }

  // 循环解析 WebP Chunks
  while (!reader->isEOF()) {
    // 读取 Chunk FourCC (4 bytes, little-endian)
    uint32_t chunkId = reader->readU32LE();
    if (chunkId == 0) {
      // 读取失败，可能到达文件末尾
      break;
    }

    // 读取 Chunk Size (4 bytes, little-endian)
    uint32_t chunkSize = reader->readU32LE();
    if (chunkSize == 0 && reader->isEOF()) {
      break;
    }

    // 检查是否是 XMP chunk
    if (chunkId == WEBP_FOURCC_XMP) {
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

