//
// Image Format Implementation
//

#include "gimt/gimt_image_format.h"
#include "gimt/gimt_def.h"
#include <algorithm>
#include <fstream>
#include <vector>

namespace gimt {

// 将字符串转换为小写
static std::string toLower(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// 从文件路径推断图像格式（基于扩展名）
ImageFormat detectFormatFromPath(const std::string& filepath) {
  // 查找最后一个点的位置
  size_t dotPos = filepath.find_last_of('.');
  if (dotPos == std::string::npos) {
    return ImageFormat::UNKNOWN;
  }

  // 提取扩展名并转换为小写
  std::string ext = toLower(filepath.substr(dotPos + 1));

  // 根据扩展名判断格式
  if (ext == "jpg" || ext == "jpeg") {
    return ImageFormat::JPEG;
  } else if (ext == "png") {
    return ImageFormat::PNG;
  } else if (ext == "webp") {
    return ImageFormat::WEBP;
  } else if (ext == "heif" || ext == "heic") {
    return ImageFormat::HEIF;
  }

  return ImageFormat::UNKNOWN;
}

// 从文件内容推断图像格式（基于文件头魔数）
ImageFormat detectFormatFromContent(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    return ImageFormat::UNKNOWN;
  }

  // 读取前 12 个字节用于格式检测
  std::vector<uint8_t> header(12, 0);
  file.read(reinterpret_cast<char*>(header.data()), 12);
  size_t bytesRead = file.gcount();
  file.close();

  if (bytesRead < 2) {
    return ImageFormat::UNKNOWN;
  }

  // 检测 JPEG (FF D8)
  if (header[0] == 0xFF && header[1] == 0xD8) {
    return ImageFormat::JPEG;
  }

  // 检测 PNG (89 50 4E 47 0D 0A 1A 0A)
  if (bytesRead >= 8) {
    auto pngSig = getPngSignature();
    bool isPng = true;
    for (size_t i = 0; i < 8; ++i) {
      if (header[i] != pngSig[i]) {
        isPng = false;
        break;
      }
    }
    if (isPng) {
      return ImageFormat::PNG;
    }
  }

  // 检测 WebP (RIFF....WEBP)
  if (bytesRead >= 12) {
    if (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' && header[3] == 'F' &&
        header[8] == 'W' && header[9] == 'E' && header[10] == 'B' && header[11] == 'P') {
      return ImageFormat::WEBP;
    }
  }

  // 检测 HEIF/HEIC (....ftyp)
  if (bytesRead >= 12) {
    // HEIF 文件通常以 ftyp box 开始，位于偏移 4-7
    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p') {
      // 进一步检查 brand (heic, mif1, hevc, hevx 等)
      // 这些 brand 通常在偏移 8-11
      if ((header[8] == 'h' && header[9] == 'e' && header[10] == 'i' && header[11] == 'c') ||
          (header[8] == 'm' && header[9] == 'i' && header[10] == 'f' && header[11] == '1') ||
          (header[8] == 'h' && header[9] == 'e' && header[10] == 'v')) {
        return ImageFormat::HEIF;
      }
    }
  }

  return ImageFormat::UNKNOWN;
}

// 获取格式名称字符串
std::string getFormatName(ImageFormat format) {
  switch (format) {
    case ImageFormat::JPEG:
      return "JPEG";
    case ImageFormat::PNG:
      return "PNG";
    case ImageFormat::WEBP:
      return "WebP";
    case ImageFormat::HEIF:
      return "HEIF";
    case ImageFormat::UNKNOWN:
    default:
      return "Unknown";
  }
}

} // namespace gimt





