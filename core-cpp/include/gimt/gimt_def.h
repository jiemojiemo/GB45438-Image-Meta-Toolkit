//
// Created by user on 1/25/26.
//

#ifndef GIMT_DEF_H
#define GIMT_DEF_H
#include <string>

namespace gimt {
struct AIGCInfo {
  std::string label;               // 标识
  std::string contentProducer;     // 内容生成者
  std::string produceID;           // 生成者 ID
  std::string reservedCode1;       // 预留字段 1
  std::string contentPropagator;   // 内容传播者
  std::string propagateID;         // 传播者 ID
  std::string reservedCode2;       // 预留字段 2
};
}

#endif //GIMT_DEF_H
