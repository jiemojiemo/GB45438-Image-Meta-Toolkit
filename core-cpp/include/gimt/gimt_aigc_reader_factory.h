//
// AIGC Reader Factory
// Factory class for creating appropriate AIGC readers based on image format
//

#ifndef GIMT_AIGC_READER_FACTORY_H
#define GIMT_AIGC_READER_FACTORY_H

#include "gimt_aigc_reader.h"
#include "gimt_image_format.h"
#include <memory>
#include <string>

namespace gimt {

class AIGCReaderFactory {
public:
  // 根据图像格式创建对应的 Reader
  // @param format: 图像格式
  // @return: Reader 智能指针，如果格式不支持则返回 nullptr
  static AIGCReaderPtr createReader(ImageFormat format);

  // 根据文件路径自动检测格式并创建 Reader（基于扩展名）
  // @param filepath: 文件路径
  // @return: Reader 智能指针，如果格式不支持则返回 nullptr
  static AIGCReaderPtr createReaderFromPath(const std::string& filepath);

  // 根据文件内容自动检测格式并创建 Reader（基于文件头）
  // @param filepath: 文件路径
  // @return: Reader 智能指针，如果格式不支持则返回 nullptr
  static AIGCReaderPtr createReaderFromContent(const std::string& filepath);

  // 创建 Reader 并自动调用 prepare
  // @param filepath: 文件路径
  // @param autoDetect: 是否自动检测格式（true: 基于文件头，false: 基于扩展名）
  // @return: 已准备好的 Reader 智能指针，失败返回 nullptr
  static AIGCReaderPtr createAndPrepare(const std::string& filepath, 
                                        bool autoDetect = true);
};

} // namespace gimt

#endif // GIMT_AIGC_READER_FACTORY_H


