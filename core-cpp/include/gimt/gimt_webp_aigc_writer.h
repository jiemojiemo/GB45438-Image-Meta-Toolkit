//
// WebP AIGC Writer - Writes AIGC metadata to WebP files via XMP chunks
//

#ifndef GIMT_WEBP_AIGC_WRITER_H
#define GIMT_WEBP_AIGC_WRITER_H

#include "gimt_def.h"
#include "gimt_aigc_writer.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gimt {

class WebpAIGCWriter : public IAIGCWriter {
public:
  WebpAIGCWriter() = default;

  // 准备写入：读取输入 WebP 并记录输出路径
  bool prepare(const std::string &inputFilepath, const std::string &outputFilepath) override;

  // 将 AIGCInfo 写入输出 WebP 的 XMP chunk 中
  bool writeAIGCInfo(const AIGCInfo &info) override;

  // 获取支持的图像格式
  ImageFormat getFormat() const override { return ImageFormat::WEBP; }

private:
  std::vector<uint8_t> inputData_;
  std::string outputPath_;

  // 构建 XMP chunk 数据
  std::vector<uint8_t> buildXMPChunk(const std::string& xmpContent);
  
  // 检查是否有 VP8X chunk
  bool hasVP8X() const;
  
  // 获取 VP8/VP8L 图像尺寸
  bool getImageDimensions(uint32_t& width, uint32_t& height) const;
  
  // 创建 VP8X chunk
  std::vector<uint8_t> createVP8XChunk(uint32_t width, uint32_t height, bool hasXMP) const;
};

} // namespace gimt

#endif // GIMT_WEBP_AIGC_WRITER_H

