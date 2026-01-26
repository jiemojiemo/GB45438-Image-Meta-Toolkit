//
// Implementation of JpegAIGCWriter
//

#include "gimt/gimt_jpeg_aigc_writer.h"
#include "gimt/gimt_xml_utils.h"

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
    if (data[offset] != gimt::JPEG_MARKER_PREFIX) {
      break;
    }

    uint8_t marker = data[offset + 1];
    if (marker != gimt::JPEG_APP0) {
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
  if (inputData_[0] != JPEG_MARKER_PREFIX || inputData_[1] != JPEG_SOI) {
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
  std::string json = info.toJson();
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
  out.put(static_cast<char>(JPEG_MARKER_PREFIX));
  out.put(static_cast<char>(JPEG_APP1));

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

