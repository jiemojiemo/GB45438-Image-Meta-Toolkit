//
// Created by user on 1/27/26.
//

#ifndef GIMT_WEBP_AIGC_READER_H
#define GIMT_WEBP_AIGC_READER_H
#include "gimt/gimt_def.h"
#include "gimt/gimt_aigc_reader.h"
#include "gimt_patter_matcher.h"

#include "gimt/gimt_binary_reader.h"
#include <fstream>
#include <memory>
#include <string>

namespace gimt {
class WebpAIGCReader : public IAIGCReader {
public:
  explicit WebpAIGCReader() = default;

  // 打开 WebP 文件并准备读取元数据（以二进制模式）
  bool prepare(const std::string& filepath) override;

  // 从 WebP 的 XMP 中解析 AIGC 信息；成功解析返回 true，否则返回 false
  bool readAIGCInfo(gimt::AIGCInfo& info) override;

  // 获取支持的图像格式
  ImageFormat getFormat() const override { return ImageFormat::WEBP; }

private:
  std::ifstream stream;
  std::unique_ptr<BinaryReader> reader{nullptr};
};
}

#endif //GIMT_WEBP_AIGC_READER_H

