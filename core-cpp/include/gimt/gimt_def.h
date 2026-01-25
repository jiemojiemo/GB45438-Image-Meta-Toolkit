//
// Created by user on 1/25/26.
//

#ifndef GIMT_DEF_H
#define GIMT_DEF_H
#include <string>

namespace gimt {
inline std::string getJsonValue(const std::string& json, const std::string& key) {
  // 构造查找关键字，例如 "Label":"
  std::string searchKey = "\"" + key + "\":\"";
  size_t startPos = json.find(searchKey);

  if (startPos == std::string::npos) {
    return ""; // 没找到 Key
  }

  // 移动到 Value 开始的位置
  startPos += searchKey.length();

  // 找到 Value 结束的引号
  size_t endPos = json.find("\"", startPos);
  if (endPos == std::string::npos) {
    return ""; // 格式异常
  }

  return json.substr(startPos, endPos - startPos);
}

class AIGCInfo {
public:
  std::string label;             // 标识
  std::string contentProducer;   // 内容生成者
  std::string produceID;         // 生成者 ID
  std::string reservedCode1;     // 预留字段 1
  std::string contentPropagator; // 内容传播者
  std::string propagateID;       // 传播者 ID
  std::string reservedCode2;     // 预留字段 2

  static void parseJsonToStruct(const std::string& jsonStr, AIGCInfo& info) {
    // 注意：JSON 中的 Key 是首字母大写的（根据你的 Extracted JSON 示例）
    info.label             = getJsonValue(jsonStr, "Label");
    info.contentProducer   = getJsonValue(jsonStr, "ContentProducer");
    info.produceID         = getJsonValue(jsonStr, "ProduceID");
    info.reservedCode1     = getJsonValue(jsonStr, "ReservedCode1");
    info.contentPropagator = getJsonValue(jsonStr, "ContentPropagator");
    info.propagateID       = getJsonValue(jsonStr, "PropagateID");
    info.reservedCode2     = getJsonValue(jsonStr, "ReservedCode2");
  }

  bool operator==(const AIGCInfo &info) const {
    return label == info.label && contentProducer == info.contentProducer &&
           produceID == info.produceID && reservedCode1 == info.reservedCode1 &&
           contentPropagator == info.contentPropagator &&
           propagateID == info.propagateID &&
           reservedCode2 == info.reservedCode2;
  }
  bool operator!=(const AIGCInfo &info) const {
    return !(*this == info);
  }

};
} // namespace gimt

#endif // GIMT_DEF_H
