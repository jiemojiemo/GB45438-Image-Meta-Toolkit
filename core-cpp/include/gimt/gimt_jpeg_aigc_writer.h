//
// Created by user on 1/25/26.
//

#ifndef GIMT_JPEG_AIGC_WRITER_H
#define GIMT_JPEG_AIGC_WRITER_H

#include "gimt_def.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gimt {

class JpegAIGCWriter {
public:
  JpegAIGCWriter() = default;

  // 准备写入：读取输入 JPEG 并记录输出路径
  bool prepare(const std::string &inputFilepath, const std::string &outputFilepath);

  // 将 AIGCInfo 写入输出 JPEG 的 XMP APP1 段中
  bool writeAIGCInfo(const AIGCInfo &info);

private:
  std::vector<uint8_t> inputData_;
  std::string outputPath_;
  bool prepared_{false};
};

} // namespace gimt

#endif // GIMT_JPEG_AIGC_WRITER_H

