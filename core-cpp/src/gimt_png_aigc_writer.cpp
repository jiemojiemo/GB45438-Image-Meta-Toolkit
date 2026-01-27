//
// Implementation of PngAIGCWriter
//

#include "gimt/gimt_png_aigc_writer.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"
#include "gimt/gimt_binary_writer.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>
#include <zlib.h>

namespace gimt {

uint32_t PngAIGCWriter::calculateCRC32(const uint8_t* data, size_t length) {
  return crc32(0, data, static_cast<uInt>(length));
}

std::vector<uint8_t> PngAIGCWriter::buildITXtChunk(const std::string& xmpContent) {
  std::vector<uint8_t> chunk;
  
  // iTXt chunk 数据部分结构：
  // 1. Keyword: "XML:com.adobe.xmp" + null terminator
  const std::string keyword = PNG_XMP_KEYWORD;
  chunk.insert(chunk.end(), keyword.begin(), keyword.end());
  chunk.push_back(0); // null terminator
  
  // 2. Compression flag: 0 (不压缩)
  chunk.push_back(0);
  
  // 3. Compression method: 0
  chunk.push_back(0);
  
  // 4. Language tag: empty + null terminator
  chunk.push_back(0);
  
  // 5. Translated keyword: empty + null terminator
  chunk.push_back(0);
  
  // 6. XMP content
  chunk.insert(chunk.end(), xmpContent.begin(), xmpContent.end());
  
  // 构建完整的 chunk：[Length][Type][Data][CRC]
  std::vector<uint8_t> fullChunk;
  
  // Length (4 bytes, big-endian) - 不包括 length 和 CRC 字段
  uint32_t dataLength = static_cast<uint32_t>(chunk.size());
  BinaryWriter::writeU32BE(fullChunk, dataLength);
  
  // Type (4 bytes): "iTXt"
  fullChunk.push_back('i');
  fullChunk.push_back('T');
  fullChunk.push_back('X');
  fullChunk.push_back('t');
  
  // Data
  fullChunk.insert(fullChunk.end(), chunk.begin(), chunk.end());
  
  // CRC (4 bytes) - 对 Type + Data 计算
  uint32_t crc = calculateCRC32(fullChunk.data() + 4, 4 + dataLength);
  BinaryWriter::writeU32BE(fullChunk, crc);
  
  return fullChunk;
}

bool PngAIGCWriter::prepare(const std::string &inputFilepath, const std::string &outputFilepath) {
  prepared_ = false;
  inputData_.clear();
  outputPath_.clear();

  std::ifstream in(inputFilepath, std::ios::binary);
  if (!in.is_open()) {
    return false;
  }

  inputData_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  in.close();

  if (inputData_.size() < 8) {
    return false;
  }

  // 校验 PNG 签名
  auto pngSig = getPngSignature();
  if (!PatternMatcher::match(inputData_.data(), 8, pngSig)) {
    return false;
  }

  outputPath_ = outputFilepath;
  prepared_ = true;
  return true;
}

bool PngAIGCWriter::writeAIGCInfo(const AIGCInfo &info) {
  if (!prepared_) {
    return false;
  }

  std::ofstream out(outputPath_, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) {
    return false;
  }

  // 构造 XMP 内容
  std::string json = info.toJson();
  std::string escapedJson = xmlEscape(json);
  std::string xmpPayload = buildXmpPayload(escapedJson);

  // 构建 iTXt chunk
  std::vector<uint8_t> itxtChunk = buildITXtChunk(xmpPayload);

  // 写入 PNG 签名
  out.write(reinterpret_cast<const char*>(inputData_.data()), 8);

  // 查找第一个 IDAT chunk 的位置，在其之前插入 iTXt chunk
  // 同时跳过已存在的 XMP iTXt chunks
  size_t offset = 8;
  bool itxtInserted = false;
  
  while (offset + 8 <= inputData_.size()) {
    // 读取 chunk length
    uint32_t chunkLength = (static_cast<uint32_t>(inputData_[offset]) << 24) |
                          (static_cast<uint32_t>(inputData_[offset + 1]) << 16) |
                          (static_cast<uint32_t>(inputData_[offset + 2]) << 8) |
                          static_cast<uint32_t>(inputData_[offset + 3]);
    
    // 读取 chunk type
    uint8_t chunkType[4];
    chunkType[0] = inputData_[offset + 4];
    chunkType[1] = inputData_[offset + 5];
    chunkType[2] = inputData_[offset + 6];
    chunkType[3] = inputData_[offset + 7];
    
    size_t chunkSize = 4 + 4 + chunkLength + 4; // length + type + data + crc
    if (offset + chunkSize > inputData_.size()) {
      break;
    }
    
    // 检查是否是 iTXt chunk 且包含 XMP keyword
    bool isXmpITXt = false;
    if (PatternMatcher::match(chunkType, 4, getPngChunkITXt())) {
      // 检查 keyword 是否是 XMP
      if (offset + 8 + PNG_XMP_KEYWORD_LEN < inputData_.size()) {
        if (PatternMatcher::matchString(&inputData_[offset + 8], chunkLength, PNG_XMP_KEYWORD)) {
          isXmpITXt = true;
        }
      }
    }
    
    // 检查是否到达 IDAT chunk
    if (chunkType[0] == 'I' && chunkType[1] == 'D' && 
        chunkType[2] == 'A' && chunkType[3] == 'T') {
      // 在 IDAT 之前插入新的 iTXt chunk
      out.write(reinterpret_cast<const char*>(itxtChunk.data()), 
                static_cast<std::streamsize>(itxtChunk.size()));
      itxtInserted = true;
      
      // 写入剩余的原始数据（包括这个 IDAT 和后续所有 chunks）
      out.write(reinterpret_cast<const char*>(&inputData_[offset]),
                static_cast<std::streamsize>(inputData_.size() - offset));
      break;
    }
    
    // 如果是旧的 XMP iTXt chunk，跳过不写入（删除旧的 XMP）
    if (!isXmpITXt) {
      // 写入当前 chunk（非 XMP iTXt）
      out.write(reinterpret_cast<const char*>(&inputData_[offset]),
                static_cast<std::streamsize>(chunkSize));
    }
    
    offset += chunkSize;
  }
  
  // 如果没有找到 IDAT（不太可能），在文件末尾插入
  if (!itxtInserted) {
    out.write(reinterpret_cast<const char*>(itxtChunk.data()), 
              static_cast<std::streamsize>(itxtChunk.size()));
  }

  return out.good();
}

} // namespace gimt
