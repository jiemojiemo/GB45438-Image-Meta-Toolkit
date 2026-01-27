//
// PNG AIGC Writer - Writes AIGC metadata to PNG files via XMP iTXt chunks
//

#ifndef GIMT_PNG_AIGC_WRITER_H
#define GIMT_PNG_AIGC_WRITER_H

#include "gimt_def.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gimt {

class PngAIGCWriter {
public:
  PngAIGCWriter() = default;

  // 准备写入：读取输入 PNG 并记录输出路径
  bool prepare(const std::string &inputFilepath, const std::string &outputFilepath);

  // 将 AIGCInfo 写入输出 PNG 的 XMP iTXt 块中
  bool writeAIGCInfo(const AIGCInfo &info);

private:
  std::vector<uint8_t> inputData_;
  std::string outputPath_;
  bool prepared_{false};

  // 计算 PNG CRC32 校验和
  uint32_t calculateCRC32(const uint8_t* data, size_t length);
  
  // 构建 iTXt chunk 数据
  std::vector<uint8_t> buildITXtChunk(const std::string& xmpContent);
};

} // namespace gimt

#endif // GIMT_PNG_AIGC_WRITER_H
