//
// AIGC Reader Factory Implementation
//

#include "gimt/gimt_aigc_reader_factory.h"
#include "gimt/gimt_jpeg_aigc_reader.h"
#include "gimt/gimt_png_aigc_reader.h"
#include "gimt/gimt_webp_aigc_reader.h"
#include "gimt/gimt_heif_aigc_reader.h"

namespace gimt {

// 根据图像格式创建对应的 Reader
AIGCReaderPtr AIGCReaderFactory::createReader(ImageFormat format) {
  switch (format) {
    case ImageFormat::JPEG:
      return std::make_unique<JpegAIGCReader>();
    case ImageFormat::PNG:
      return std::make_unique<PngAIGCReader>();
    case ImageFormat::WEBP:
      return std::make_unique<WebpAIGCReader>();
    case ImageFormat::HEIF:
      return std::make_unique<HeifAIGCReader>();
    case ImageFormat::UNKNOWN:
    default:
      return nullptr;
  }
}

// 根据文件路径自动检测格式并创建 Reader（基于扩展名）
AIGCReaderPtr AIGCReaderFactory::createReaderFromPath(const std::string& filepath) {
  ImageFormat format = detectFormatFromPath(filepath);
  return createReader(format);
}

// 根据文件内容自动检测格式并创建 Reader（基于文件头）
AIGCReaderPtr AIGCReaderFactory::createReaderFromContent(const std::string& filepath) {
  ImageFormat format = detectFormatFromContent(filepath);
  return createReader(format);
}

// 创建 Reader 并自动调用 prepare
AIGCReaderPtr AIGCReaderFactory::createAndPrepare(const std::string& filepath, 
                                                   bool autoDetect) {
  // 根据选项检测格式
  AIGCReaderPtr reader = autoDetect 
    ? createReaderFromContent(filepath)
    : createReaderFromPath(filepath);

  if (!reader) {
    return nullptr;
  }

  // 调用 prepare
  if (!reader->prepare(filepath)) {
    return nullptr;
  }

  return reader;
}

} // namespace gimt





