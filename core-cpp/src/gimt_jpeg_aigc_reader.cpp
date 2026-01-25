//
// Implementation of JpegAIGCReader
//

#include "gimt/gimt_jpeg_aigc_reader.h"

#include <vector>

namespace gimt {
namespace {

// 简单的 XML entity 反转义，只覆盖当前使用到的几种
std::string xmlUnescape(std::string str) {
  static const std::pair<std::string, std::string> entities[] = {
      {"&quot;", "\""},
      {"&amp;", "&"},
      {"&lt;", "<"},
      {"&gt;", ">"},
      {"&apos;", "'"}};

  size_t start_pos = 0;
  for (const auto &entity : entities) {
    start_pos = 0;
    while ((start_pos = str.find(entity.first, start_pos)) != std::string::npos) {
      str.replace(start_pos, entity.first.length(), entity.second);
      start_pos += entity.second.length();
    }
  }
  return str;
}

constexpr const char XMP_SIGNATURE[] = "http://ns.adobe.com/xap/1.0/";

bool extractAigcJsonFromXmp(const std::string &xmpStr, std::string &outJson) {
  const std::string targetKey = "TC260:AIGC=\"";
  size_t startPos = xmpStr.find(targetKey);
  if (startPos == std::string::npos) {
    return false;
  }

  startPos += targetKey.length();
  size_t endPos = xmpStr.find("\"", startPos);
  if (endPos == std::string::npos || endPos <= startPos) {
    return false;
  }

  std::string escapedJson = xmpStr.substr(startPos, endPos - startPos);
  outJson = xmlUnescape(escapedJson);
  return true;
}

} // namespace

bool JpegAIGCReader::prepare(const std::string &filepath) {
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

bool JpegAIGCReader::readAIGCInfo(gimt::AIGCInfo &info) {
  if (!reader) {
    return false;
  }

  // 每次读取前将流重置到开头，保证重复调用的一致性
  stream.clear();
  stream.seekg(0, std::ios::beg);

  uint8_t soi[2];
  if (reader->readBytes(soi, 2) != 2) {
    return false;
  }
  if (soi[0] != 0xFF || soi[1] != 0xD8) {
    // Not a JPEG
    return false;
  }

  // 循环解析 Marker
  while (!reader->isEOF()) {
    uint8_t marker[2];
    if (reader->readBytes(marker, 2) != 2) {
      // 文件意外结束
      return false;
    }

    // 所有的 JPEG Marker 都以 0xFF 开头
    if (marker[0] != 0xFF) {
      return false;
    }

    // SOS (Start of Scan) 标志着元数据区结束，图像数据开始
    if (marker[1] == 0xDA) {
      break;
    }

    // 读取段长度 (大端序, 包含长度字段自身的 2 字节)
    uint16_t segmentLen = reader->readU16BE();
    if (segmentLen < 2) {
      return false;
    }

    uint16_t payloadLen = static_cast<uint16_t>(segmentLen - 2);

    // 判断是否是 APP1 (0xE1)
    if (marker[1] == 0xE1) {
      const std::string xmpSigStr(XMP_SIGNATURE);
      const size_t sigLen = xmpSigStr.size();

      if (payloadLen <= sigLen) {
        // 长度不足以包含 XMP 签名和内容
        reader->skip(payloadLen);
        continue;
      }

      std::vector<uint8_t> sigBuf(sigLen);
      if (reader->readBytes(sigBuf.data(), sigLen) != sigLen) {
        return false;
      }

      if (PatternMatcher::matchString(sigBuf.data(), sigLen, xmpSigStr)) {
        size_t xmpDataLen = payloadLen - sigLen;
        std::vector<uint8_t> xmpContent(xmpDataLen);
        if (reader->readBytes(xmpContent.data(), xmpDataLen) != xmpDataLen) {
          return false;
        }

        if (xmpContent.size() < 2 || xmpContent[0] != '\0') {
          return false;
        }

        std::string xmpStr(reinterpret_cast<const char *>(xmpContent.data() + 1),
                           static_cast<size_t>(xmpContent.size() - 1));
        if (xmpStr.find("<x:xmpmeta") == std::string::npos) {
          return false;
        }
        std::string pureJson;
        if (!extractAigcJsonFromXmp(xmpStr, pureJson)) {
          return false;
        }

        AIGCInfo::parseJsonToStruct(pureJson, info);
        return true;
      } else {
        // 是 APP1 但不是 XMP (可能是 Exif)，跳过已读取签名后的剩余部分
        size_t remaining = payloadLen - sigLen;
        reader->skip(remaining);
      }
    } else {
      // 其他段，直接跳过 (长度字段已读，还需跳 payloadLen)
      reader->skip(payloadLen);
    }
  }

  return false;
}

} // namespace gimt

