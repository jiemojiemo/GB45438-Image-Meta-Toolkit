//
// Created by user on 1/25/26.
//

#ifndef GIMT_DEF_H
#define GIMT_DEF_H
#include <cstdint>
#include <string>
#include <vector>

namespace gimt {

// JPEG Marker Constants
constexpr uint8_t JPEG_MARKER_PREFIX = 0xFF;
constexpr uint8_t JPEG_SOI = 0xD8;   // Start of Image
constexpr uint8_t JPEG_SOS = 0xDA;   // Start of Scan
constexpr uint8_t JPEG_APP0 = 0xE0;  // APP0 marker
constexpr uint8_t JPEG_APP1 = 0xE1;  // APP1 marker

// PNG File Signature (8 bytes)
inline std::vector<uint8_t> getPngSignature() {
  return {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
}

// PNG Chunk Types
inline std::vector<uint8_t> getPngChunkIEND() {
  return {'I', 'E', 'N', 'D'};
}

inline std::vector<uint8_t> getPngChunkITXt() {
  return {'i', 'T', 'X', 't'};
}

// XMP Signatures
constexpr const char XMP_SIGNATURE[] = "http://ns.adobe.com/xap/1.0/";
constexpr const char PNG_XMP_KEYWORD[] = "XML:com.adobe.xmp";
constexpr size_t PNG_XMP_KEYWORD_LEN = 17; // length of "XML:com.adobe.xmp" without null terminator

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

  // Parse JSON string to populate AIGCInfo struct
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

  // Build JSON string from AIGCInfo struct
  std::string toJson() const {
    std::string json = "{";
    json += "\"Label\":\"" + label + "\"";
    json += ",\"ContentProducer\":\"" + contentProducer + "\"";
    json += ",\"ProduceID\":\"" + produceID + "\"";
    json += ",\"ReservedCode1\":\"" + reservedCode1 + "\"";
    json += ",\"ContentPropagator\":\"" + contentPropagator + "\"";
    json += ",\"PropagateID\":\"" + propagateID + "\"";
    json += ",\"ReservedCode2\":\"" + reservedCode2 + "\"";
    json += "}";
    return json;
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
