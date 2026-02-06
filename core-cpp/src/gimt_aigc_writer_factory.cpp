//
// AIGC Writer Factory Implementation
//

#include "gimt/gimt_aigc_writer_factory.h"
#include "gimt/gimt_jpeg_aigc_writer.h"
#include "gimt/gimt_png_aigc_writer.h"
#include "gimt/gimt_webp_aigc_writer.h"
#include "gimt/gimt_heif_aigc_writer.h"

namespace gimt {

// 根据图像格式创建对应的 Writer
AIGCWriterPtr AIGCWriterFactory::createWriter(ImageFormat format) {
  switch (format) {
    case ImageFormat::JPEG:
      return std::make_unique<JpegAIGCWriter>();
    case ImageFormat::PNG:
      return std::make_unique<PngAIGCWriter>();
    case ImageFormat::WEBP:
      return std::make_unique<WebpAIGCWriter>();
    case ImageFormat::HEIF:
      return std::make_unique<HeifAIGCWriter>();
    case ImageFormat::UNKNOWN:
    default:
      return nullptr;
  }
}

// 根据文件路径自动检测格式并创建 Writer（基于扩展名）
AIGCWriterPtr AIGCWriterFactory::createWriterFromPath(const std::string& filepath) {
  ImageFormat format = detectFormatFromPath(filepath);
  return createWriter(format);
}

// 根据文件内容自动检测格式并创建 Writer（基于文件头）
AIGCWriterPtr AIGCWriterFactory::createWriterFromContent(const std::string& filepath) {
  ImageFormat format = detectFormatFromContent(filepath);
  return createWriter(format);
}

// 创建 Writer 并自动调用 prepare
AIGCWriterPtr AIGCWriterFactory::createAndPrepare(const std::string& inputFilepath,
                                                   const std::string& outputFilepath,
                                                   bool autoDetect) {
  // 根据选项检测格式
  AIGCWriterPtr writer = autoDetect 
    ? createWriterFromContent(inputFilepath)
    : createWriterFromPath(inputFilepath);

  if (!writer) {
    return nullptr;
  }

  // 调用 prepare
  if (!writer->prepare(inputFilepath, outputFilepath)) {
    return nullptr;
  }

  return writer;
}

} // namespace gimt

