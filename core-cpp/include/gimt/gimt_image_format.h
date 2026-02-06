//
// Image Format Enumeration
// Defines supported image formats for AIGC metadata operations
//

#ifndef GIMT_IMAGE_FORMAT_H
#define GIMT_IMAGE_FORMAT_H

#include <string>

namespace gimt {

// 支持的图像格式枚举
enum class ImageFormat {
  JPEG,
  PNG,
  WEBP,
  HEIF,
  UNKNOWN
};

// 从文件路径推断图像格式（基于扩展名）
ImageFormat detectFormatFromPath(const std::string& filepath);

// 从文件内容推断图像格式（基于文件头魔数）
ImageFormat detectFormatFromContent(const std::string& filepath);

// 获取格式名称字符串
std::string getFormatName(ImageFormat format);

} // namespace gimt

#endif // GIMT_IMAGE_FORMAT_H

