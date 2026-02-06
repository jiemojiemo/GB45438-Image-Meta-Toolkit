//
// AIGC Reader Interface
// Abstract base class for all AIGC metadata readers
//

#ifndef GIMT_AIGC_READER_H
#define GIMT_AIGC_READER_H

#include "gimt_def.h"
#include "gimt_image_format.h"
#include <memory>
#include <string>

namespace gimt {

// AIGC Reader 抽象基类
class IAIGCReader {
public:
  virtual ~IAIGCReader() = default;

  // 准备读取：打开文件并初始化
  // @param filepath: 输入文件路径
  // @return: 成功返回 true，失败返回 false
  virtual bool prepare(const std::string& filepath) = 0;

  // 读取 AIGC 元数据信息
  // @param info: 输出参数，存储读取到的 AIGC 信息
  // @return: 成功返回 true，失败返回 false
  virtual bool readAIGCInfo(AIGCInfo& info) = 0;

  // 获取当前 Reader 支持的图像格式
  virtual ImageFormat getFormat() const = 0;

  // 检查文件是否已准备好
  virtual bool isPrepared() const { return prepared_; }

protected:
  bool prepared_ = false;
};

// 智能指针类型别名
using AIGCReaderPtr = std::unique_ptr<IAIGCReader>;

} // namespace gimt

#endif // GIMT_AIGC_READER_H

