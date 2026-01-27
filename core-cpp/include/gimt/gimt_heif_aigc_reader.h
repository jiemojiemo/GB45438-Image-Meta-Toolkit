//
// Created by user on 1/27/26.
//

#ifndef GIMT_HEIF_AIGC_READER_H
#define GIMT_HEIF_AIGC_READER_H
#include "gimt/gimt_def.h"
#include "gimt_patter_matcher.h"

#include "gimt/gimt_binary_reader.h"
#include <fstream>
#include <memory>
#include <string>

namespace gimt {
class HeifAIGCReader {
public:
  explicit HeifAIGCReader() = default;

  // 打开 HEIF/HEIC 文件并准备读取元数据（以二进制模式）
  bool prepare(const std::string& filepath);

  // 从 HEIF/HEIC 的 XMP 中解析 AIGC 信息；成功解析返回 true，否则返回 false
  bool readAIGCInfo(gimt::AIGCInfo& info);

private:
  std::ifstream stream;
  std::unique_ptr<BinaryReader> reader{nullptr};
};
}

#endif //GIMT_HEIF_AIGC_READER_H

