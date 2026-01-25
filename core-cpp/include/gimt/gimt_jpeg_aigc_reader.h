//
// Created by user on 1/25/26.
//

#ifndef GIMT_JPEG_AIGC_READER_H
#define GIMT_JPEG_AIGC_READER_H
#include "gimt/gimt_def.h"
#include "gimt_patter_matcher.h"

#include "gimt/gimt_binary_reader.h"
#include <fstream>
#include <memory>
#include <string>

namespace gimt {
class JpegAIGCReader {
public:
  explicit JpegAIGCReader() = default;

  // 打开 JPEG 文件并准备读取元数据（以二进制模式）
  bool prepare(const std::string& filepath);

  // 从 JPEG 的 XMP 中解析 AIGC 信息；成功解析返回 true，否则返回 false
  bool readAIGCInfo(gimt::AIGCInfo& info);

private:
  std::ifstream stream;
  std::unique_ptr<BinaryReader> reader{nullptr};
};
}

#endif //GIMT_JPEG_AIGC_READER_H
