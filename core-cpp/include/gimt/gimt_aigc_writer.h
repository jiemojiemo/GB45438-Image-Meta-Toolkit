//
// AIGC Writer Interface
// Abstract base class for all AIGC metadata writers
//

#ifndef GIMT_AIGC_WRITER_H
#define GIMT_AIGC_WRITER_H

#include "gimt_def.h"
#include "gimt_image_format.h"
#include <memory>
#include <string>

namespace gimt {

// AIGC Writer 抽象基类
class IAIGCWriter {
public:
  virtual ~IAIGCWriter() = default;

  // 准备写入：读取输入文件并设置输出路径
  // @param inputFilepath: 输入文件路径
  // @param outputFilepath: 输出文件路径
  // @return: 成功返回 true，失败返回 false
  virtual bool prepare(const std::string& inputFilepath, 
                      const std::string& outputFilepath) = 0;

  // 写入 AIGC 元数据信息
  // @param info: 要写入的 AIGC 信息
  // @return: 成功返回 true，失败返回 false
  virtual bool writeAIGCInfo(const AIGCInfo& info) = 0;

  // 获取当前 Writer 支持的图像格式
  virtual ImageFormat getFormat() const = 0;

  // 检查文件是否已准备好
  virtual bool isPrepared() const { return prepared_; }

protected:
  bool prepared_ = false;
};

// 智能指针类型别名
using AIGCWriterPtr = std::unique_ptr<IAIGCWriter>;

} // namespace gimt

#endif // GIMT_AIGC_WRITER_H


