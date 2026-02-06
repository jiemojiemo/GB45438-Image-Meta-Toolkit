//
// Created by user on 1/26/26.
//

#ifndef GIMT_PNG_AIGC_READER_H
#define GIMT_PNG_AIGC_READER_H
#include "gimt/gimt_def.h"
#include "gimt/gimt_aigc_reader.h"
#include "gimt_patter_matcher.h"

#include "gimt/gimt_binary_reader.h"
#include <fstream>
#include <memory>
#include <string>

namespace gimt {
class PngAIGCReader : public IAIGCReader {
public:
  explicit PngAIGCReader() = default;

  // 打开 PNG 文件并准备读取元数据（以二进制模式）
  bool prepare(const std::string& filepath) override;

  // 从 PNG 的 XMP 中解析 AIGC 信息；成功解析返回 true，否则返回 false
  bool readAIGCInfo(gimt::AIGCInfo& info) override;

  // 获取支持的图像格式
  ImageFormat getFormat() const override { return ImageFormat::PNG; }

private:
  std::ifstream stream;
  std::unique_ptr<BinaryReader> reader{nullptr};
};
}

#endif //GIMT_PNG_AIGC_READER_H
