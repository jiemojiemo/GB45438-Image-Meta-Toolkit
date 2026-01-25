//
// Implementation of JpegAIGCWriter
//

#include "gimt/gimt_jpeg_aigc_writer.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

namespace {

size_t findApp0EndOffset(const std::vector<uint8_t> &data) {
  size_t offset = 2;
  size_t app0End = 2;
  const size_t dataSize = data.size();
  while (offset + 3 < dataSize) {
    if (data[offset] != 0xFF) {
      break;
    }

    uint8_t marker = data[offset + 1];
    if (marker != 0xE0) {
      break;
    }

    uint16_t segmentLen =
        (static_cast<uint16_t>(data[offset + 2]) << 8) | data[offset + 3];
    if (segmentLen < 2) {
      break;
    }

    size_t next = offset + 2 + segmentLen;
    if (next > dataSize) {
      break;
    }

    app0End = next;
    offset = next;
  }
  return app0End;
}

} // namespace

namespace gimt {
namespace {

constexpr const char XMP_SIGNATURE[] = "http://ns.adobe.com/xap/1.0/";

std::string buildXmpPayload(const std::string &escapedJson) {
  std::string payload;
  payload.reserve(256);
  payload += "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">";
  payload += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
             "xmlns:TC260=\"http://ns.adobe.com/xap/1.0/tc/\">";
  payload += "<rdf:Description rdf:about=\"\" TC260:AIGC=\"";
  payload += escapedJson;
  payload += "\"/>";
  payload += "</rdf:RDF>";
  payload += "</x:xmpmeta>";
  return payload;
}

// 简单 XML entity 转义，与读取端的 xmlUnescape 互补
std::string xmlEscape(const std::string &str) {
  std::string result;
  result.reserve(str.size());

  for (char ch : str) {
    switch (ch) {
    case '&':
      result += "&amp;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    case '"':
      result += "&quot;";
      break;
    case '\'':
      result += "&apos;";
      break;
    default:
      result += ch;
      break;
    }
  }

  return result;
}

std::string buildAigcJson(const AIGCInfo &info) {
  // 与 AIGCInfo::parseJsonToStruct 中使用的 Key 保持一致
  std::string json = "{";
  json += "\"Label\":\"" + info.label + "\"";
  json += ",\"ContentProducer\":\"" + info.contentProducer + "\"";
  json += ",\"ProduceID\":\"" + info.produceID + "\"";
  json += ",\"ReservedCode1\":\"" + info.reservedCode1 + "\"";
  json += ",\"ContentPropagator\":\"" + info.contentPropagator + "\"";
  json += ",\"PropagateID\":\"" + info.propagateID + "\"";
  json += ",\"ReservedCode2\":\"" + info.reservedCode2 + "\"";
  json += "}";
  return json;
}

} // namespace

bool JpegAIGCWriter::prepare(const std::string &inputFilepath, const std::string &outputFilepath) {
  prepared_ = false;
  inputData_.clear();
  outputPath_.clear();

  std::ifstream in(inputFilepath, std::ios::binary);
  if (!in.is_open()) {
    return false;
  }

  inputData_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  in.close();

  if (inputData_.size() < 2) {
    return false;
  }

  // 校验 JPEG SOI
  if (inputData_[0] != 0xFF || inputData_[1] != 0xD8) {
    return false;
  }

  outputPath_ = outputFilepath;
  prepared_ = true;
  return true;
}

bool JpegAIGCWriter::writeAIGCInfo(const AIGCInfo &info) {
  if (!prepared_) {
    return false;
  }

  std::ofstream out(outputPath_, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) {
    return false;
  }

  if (inputData_.size() < 2) {
    return false;
  }

  // 写入 SOI
  out.put(static_cast<char>(inputData_[0]));
  out.put(static_cast<char>(inputData_[1]));
  const std::string xmpSigStr(XMP_SIGNATURE);
  const size_t sigLen = xmpSigStr.size();

  // 构造 JSON + XML 转义后的 AIGC 字段
  std::string json = buildAigcJson(info);
  std::string escapedJson = xmlEscape(json);
  std::string xmpPayload = buildXmpPayload(escapedJson);

  const size_t payloadLen = sigLen + 1 + xmpPayload.size(); // +1 用于空终止符
  if (payloadLen > static_cast<size_t>(std::numeric_limits<uint16_t>::max() - 2)) {
    return false;
  }
  const uint16_t segmentLen = static_cast<uint16_t>(payloadLen + 2);

  const size_t app0End = findApp0EndOffset(inputData_);

  // 写入 APP1 标记之前先写入原有的 APP0
  if (app0End > 2) {
    const size_t prefixLen = app0End - 2;
    out.write(reinterpret_cast<const char *>(&inputData_[2]),
              static_cast<std::streamsize>(prefixLen));
  }

  // 写入 APP1 标记
  out.put(static_cast<char>(0xFF));
  out.put(static_cast<char>(0xE1));

  // 写入段长度（大端）
  out.put(static_cast<char>((segmentLen >> 8) & 0xFF));
  out.put(static_cast<char>(segmentLen & 0xFF));

  // 写入 XMP 签名
  out.write(xmpSigStr.data(), static_cast<std::streamsize>(sigLen));

  // 写入空终止符直至 XMP 内容
  out.put(static_cast<char>(0));

  // 写入 XMP 内容
  out.write(xmpPayload.data(), static_cast<std::streamsize>(xmpPayload.size()));

  // 写回原始 JPEG 中剩余数据（剔除已写入的 APP0 前缀）
  if (inputData_.size() > app0End) {
    out.write(reinterpret_cast<const char *>(&inputData_[app0End]),
              static_cast<std::streamsize>(inputData_.size() - app0End));
  }

  return out.good();
}

} // namespace gimt

