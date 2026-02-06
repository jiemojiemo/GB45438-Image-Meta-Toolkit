//
// AIGC Writer Factory
// Factory class for creating appropriate AIGC writers based on image format
//

#ifndef GIMT_AIGC_WRITER_FACTORY_H
#define GIMT_AIGC_WRITER_FACTORY_H

#include "gimt_aigc_writer.h"
#include "gimt_image_format.h"
#include <memory>
#include <string>

namespace gimt {

class AIGCWriterFactory {
public:
  // 根据图像格式创建对应的 Writer
  // @param format: 图像格式
  // @return: Writer 智能指针，如果格式不支持则返回 nullptr
  static AIGCWriterPtr createWriter(ImageFormat format);

  // 根据文件路径自动检测格式并创建 Writer（基于扩展名）
  // @param filepath: 文件路径
  // @return: Writer 智能指针，如果格式不支持则返回 nullptr
  static AIGCWriterPtr createWriterFromPath(const std::string& filepath);

  // 根据文件内容自动检测格式并创建 Writer（基于文件头）
  // @param filepath: 文件路径
  // @return: Writer 智能指针，如果格式不支持则返回 nullptr
  static AIGCWriterPtr createWriterFromContent(const std::string& filepath);

  // 创建 Writer 并自动调用 prepare
  // @param inputFilepath: 输入文件路径
  // @param outputFilepath: 输出文件路径
  // @param autoDetect: 是否自动检测格式（true: 基于文件头，false: 基于扩展名）
  // @return: 已准备好的 Writer 智能指针，失败返回 nullptr
  static AIGCWriterPtr createAndPrepare(const std::string& inputFilepath,
                                        const std::string& outputFilepath,
                                        bool autoDetect = true);
};

} // namespace gimt

#endif // GIMT_AIGC_WRITER_FACTORY_H

