//
// Created by user on 1/25/26.
//

#ifndef GIMT_JPEG_AIGC_READER_H
#define GIMT_JPEG_AIGC_READER_H
#include "gimt/gimt_def.h"
#include "gimt_patter_matcher.h"

#include "gimt/gimt_binary_reader.h"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace gimt {
std::string xmlUnescape(std::string str) {
  static const std::pair<std::string, std::string> entities[] = {
    {"&quot;", "\""},
    {"&amp;",  "&"},
    {"&lt;",   "<"},
    {"&gt;",   ">"},
    {"&apos;", "'"}
  };

  size_t start_pos = 0;
  for (const auto& entity : entities) {
    start_pos = 0;
    while ((start_pos = str.find(entity.first, start_pos)) != std::string::npos) {
      str.replace(start_pos, entity.first.length(), entity.second);
      start_pos += entity.second.length();
    }
  }
  return str;
}

class JpegAIGCReader {
public:
  explicit JpegAIGCReader() = default;

  bool prepare(const std::string& filepath) {
    stream.open(filepath);
    if (!stream.is_open()) {
      return false;
    }

    reader = std::make_unique<BinaryReader>(stream);

    return true;
  }

  bool readAIGCInfo(gimt::AIGCInfo& info) {
    if (reader == nullptr || reader->isEOF()) {
      return false;
    }

    uint8_t soi[2];
    reader->readBytes(soi, 2);
    if (soi[0] != 0xFF || soi[1] != 0xD8) {
      std::cerr << "Not a JPEG" << std::endl;
      return false;
    }

    // 2. 循环解析 Marker
    while (!reader->isEOF()) {
      uint8_t marker[2];
      if (!reader->readBytes(marker, 2)) break;

      // 所有的 JPEG Marker 都以 0xFF 开头
      if (marker[0] != 0xFF) break;

      // SOS (Start of Scan) 标志着元数据区结束，图像数据开始
      if (marker[1] == 0xDA) break;

      // 读取段长度 (大端序, 包含长度字段自身的 2 字节)
      uint16_t segmentLen = reader->readU16BE();
      if (segmentLen < 2) break;

      // 判断是否是 APP1 (0xE1)
      if (marker[1] == 0xE1) {
        // 尝试读取 XMP 命名空间签名 (29字节)
        const std::string xmpSig = "http://ns.adobe.com/xap/1.0/\0";
        std::vector<uint8_t> sigBuf(29);
        reader->readBytes(sigBuf.data(), 29);

        if (PatternMatcher::matchString(sigBuf.data(), 29, xmpSig)) {
          // 命中！读取剩余的 XMP 数据包
          size_t xmpDataLen = segmentLen - 2 - 29;
          std::vector<uint8_t> xmpContent(xmpDataLen);
          reader->readBytes(xmpContent.data(), xmpDataLen);
          std::string xmpStr = std::string(xmpContent.begin(), xmpContent.end());
          // --- 新增提取逻辑 ---
          std::string targetKey = "TC260:AIGC=\"";
          size_t startPos = xmpStr.find(targetKey);
          if (startPos != std::string::npos) {
            startPos += targetKey.length();
            size_t endPos = xmpStr.find("\"", startPos);
            if (endPos != std::string::npos) {
              // 1. 截取原始转义字符串
              std::string escapedJson = xmpStr.substr(startPos, endPos - startPos);

              // 2. 反转义 &quot; -> "
              std::string pureJson = xmlUnescape(escapedJson);

              std::cout << "Extracted JSON: " << pureJson << std::endl;

              AIGCInfo::parseJsonToStruct(pureJson, info);
            }
          }

          return true;
        } else {
          // 是 APP1 但不是 XMP (可能是 Exif)，跳过已读取签名后的剩余部分
          reader->skip(segmentLen - 2 - 29);
        }
      } else {
        // 其他段，直接跳过 (长度字段已读，还需跳 segmentLen - 2)
        reader->skip(segmentLen - 2);
      }
    }


    return false;
  }

private:
  std::ifstream stream;
  std::unique_ptr<BinaryReader> reader{nullptr};
};
}

#endif //GIMT_JPEG_AIGC_READER_H
