//
// Implementation of WebpAIGCWriter
//

#include "gimt/gimt_webp_aigc_writer.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"
#include "gimt/gimt_binary_writer.h"
#include "gimt/gimt_binary_reader.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

namespace gimt {

std::vector<uint8_t> WebpAIGCWriter::buildXMPChunk(const std::string& xmpContent) {
  std::vector<uint8_t> chunk;
  
  // Chunk header: FourCC 'XMP ' (4 bytes, little-endian)
  BinaryWriter::writeU32LE(chunk, WEBP_FOURCC_XMP);
  
  // Chunk size (4 bytes, little-endian)
  uint32_t payloadSize = static_cast<uint32_t>(xmpContent.size());
  BinaryWriter::writeU32LE(chunk, payloadSize);
  
  // XMP content (payload)
  chunk.insert(chunk.end(), xmpContent.begin(), xmpContent.end());
  
  // Add padding if payload size is odd (RIFF requirement)
  if (payloadSize % 2 != 0) {
    chunk.push_back(0x00);
  }
  
  return chunk;
}

bool WebpAIGCWriter::hasVP8X() const {
  if (inputData_.size() < 12 + 8) {
    return false;
  }
  
  // Check if first chunk after RIFF header is VP8X
  uint32_t firstChunkId = (static_cast<uint32_t>(inputData_[12]) |
                           (static_cast<uint32_t>(inputData_[13]) << 8) |
                           (static_cast<uint32_t>(inputData_[14]) << 16) |
                           (static_cast<uint32_t>(inputData_[15]) << 24));
  
  return firstChunkId == 0x58385056; // 'VP8X' in little-endian
}

bool WebpAIGCWriter::getImageDimensions(uint32_t& width, uint32_t& height) const {
  size_t offset = 12; // Skip RIFF header
  
  while (offset + 8 <= inputData_.size()) {
    uint32_t chunkId = (static_cast<uint32_t>(inputData_[offset]) |
                        (static_cast<uint32_t>(inputData_[offset + 1]) << 8) |
                        (static_cast<uint32_t>(inputData_[offset + 2]) << 16) |
                        (static_cast<uint32_t>(inputData_[offset + 3]) << 24));
    
    uint32_t chunkSize = (static_cast<uint32_t>(inputData_[offset + 4]) |
                          (static_cast<uint32_t>(inputData_[offset + 5]) << 8) |
                          (static_cast<uint32_t>(inputData_[offset + 6]) << 16) |
                          (static_cast<uint32_t>(inputData_[offset + 7]) << 24));
    
    // VP8X chunk
    if (chunkId == 0x58385056) {
      if (offset + 8 + 10 > inputData_.size()) {
        return false;
      }
      // Canvas width: 3 bytes at offset 4-6 (after flags and reserved)
      width = (static_cast<uint32_t>(inputData_[offset + 8 + 4]) |
               (static_cast<uint32_t>(inputData_[offset + 8 + 5]) << 8) |
               (static_cast<uint32_t>(inputData_[offset + 8 + 6]) << 16)) + 1;
      
      // Canvas height: 3 bytes at offset 7-9
      height = (static_cast<uint32_t>(inputData_[offset + 8 + 7]) |
                (static_cast<uint32_t>(inputData_[offset + 8 + 8]) << 8) |
                (static_cast<uint32_t>(inputData_[offset + 8 + 9]) << 16)) + 1;
      return true;
    }
    
    // VP8 chunk (lossy)
    if (chunkId == 0x20385056) {
      if (offset + 8 + 10 > inputData_.size()) {
        return false;
      }
      // Skip frame tag (3 bytes) and start code (3 bytes)
      // Width and height are in bytes 6-9
      uint32_t sizeCode = (static_cast<uint32_t>(inputData_[offset + 8 + 6]) |
                           (static_cast<uint32_t>(inputData_[offset + 8 + 7]) << 8) |
                           (static_cast<uint32_t>(inputData_[offset + 8 + 8]) << 16) |
                           (static_cast<uint32_t>(inputData_[offset + 8 + 9]) << 24));
      
      width = (sizeCode & 0x3FFF);
      height = ((sizeCode >> 16) & 0x3FFF);
      return true;
    }
    
    // VP8L chunk (lossless)
    if (chunkId == 0x4C385056) {
      if (offset + 8 + 5 > inputData_.size()) {
        return false;
      }
      // Skip signature byte (0x2F)
      // Width and height are encoded in next 4 bytes
      uint32_t bits = (static_cast<uint32_t>(inputData_[offset + 8 + 1]) |
                       (static_cast<uint32_t>(inputData_[offset + 8 + 2]) << 8) |
                       (static_cast<uint32_t>(inputData_[offset + 8 + 3]) << 16) |
                       (static_cast<uint32_t>(inputData_[offset + 8 + 4]) << 24));
      
      width = (bits & 0x3FFF) + 1;
      height = ((bits >> 14) & 0x3FFF) + 1;
      return true;
    }
    
    // Move to next chunk
    size_t chunkTotalSize = 8 + chunkSize;
    if (chunkSize % 2 != 0) {
      chunkTotalSize++; // Account for padding
    }
    offset += chunkTotalSize;
  }
  
  return false;
}

std::vector<uint8_t> WebpAIGCWriter::createVP8XChunk(uint32_t width, uint32_t height, bool hasXMP) const {
  std::vector<uint8_t> chunk;
  
  // Chunk header: FourCC 'VP8X' (4 bytes, little-endian)
  BinaryWriter::writeU32LE(chunk, 0x58385056);
  
  // Chunk size: always 10 bytes for VP8X
  BinaryWriter::writeU32LE(chunk, 10);
  
  // Flags byte: bit 2 = XMP metadata
  uint8_t flags = hasXMP ? 0x04 : 0x00;
  chunk.push_back(flags);
  
  // Reserved: 3 bytes
  chunk.push_back(0x00);
  chunk.push_back(0x00);
  chunk.push_back(0x00);
  
  // Canvas width - 1 (3 bytes, little-endian)
  uint32_t widthMinus1 = width - 1;
  chunk.push_back(static_cast<uint8_t>(widthMinus1 & 0xFF));
  chunk.push_back(static_cast<uint8_t>((widthMinus1 >> 8) & 0xFF));
  chunk.push_back(static_cast<uint8_t>((widthMinus1 >> 16) & 0xFF));
  
  // Canvas height - 1 (3 bytes, little-endian)
  uint32_t heightMinus1 = height - 1;
  chunk.push_back(static_cast<uint8_t>(heightMinus1 & 0xFF));
  chunk.push_back(static_cast<uint8_t>((heightMinus1 >> 8) & 0xFF));
  chunk.push_back(static_cast<uint8_t>((heightMinus1 >> 16) & 0xFF));
  
  return chunk;
}

bool WebpAIGCWriter::prepare(const std::string &inputFilepath, const std::string &outputFilepath) {
  prepared_ = false;
  inputData_.clear();
  outputPath_.clear();

  std::ifstream in(inputFilepath, std::ios::binary);
  if (!in.is_open()) {
    return false;
  }

  inputData_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  in.close();

  if (inputData_.size() < 12) {
    return false;
  }

  // 验证 RIFF header
  uint32_t riffTag = (static_cast<uint32_t>(inputData_[0]) |
                      (static_cast<uint32_t>(inputData_[1]) << 8) |
                      (static_cast<uint32_t>(inputData_[2]) << 16) |
                      (static_cast<uint32_t>(inputData_[3]) << 24));
  
  if (riffTag != WEBP_FOURCC_RIFF) {
    return false;
  }

  uint32_t webpTag = (static_cast<uint32_t>(inputData_[8]) |
                      (static_cast<uint32_t>(inputData_[9]) << 8) |
                      (static_cast<uint32_t>(inputData_[10]) << 16) |
                      (static_cast<uint32_t>(inputData_[11]) << 24));
  
  if (webpTag != WEBP_FOURCC_WEBP) {
    return false;
  }

  outputPath_ = outputFilepath;
  prepared_ = true;
  return true;
}

bool WebpAIGCWriter::writeAIGCInfo(const AIGCInfo &info) {
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

  // 构建 XMP chunk
  std::vector<uint8_t> xmpChunk = buildXMPChunk(xmpPayload);

  // 检查是否需要创建或更新 VP8X chunk
  bool needsVP8X = !hasVP8X();
  std::vector<uint8_t> vp8xChunk;
  
  if (needsVP8X) {
    // 需要创建 VP8X chunk
    uint32_t width = 0, height = 0;
    if (!getImageDimensions(width, height)) {
      return false;
    }
    vp8xChunk = createVP8XChunk(width, height, true);
  }

  // 写入 RIFF header (先占位，稍后更新文件大小)
  out.write(reinterpret_cast<const char*>(inputData_.data()), 12);

  size_t offset = 12;
  bool xmpInserted = false;
  bool vp8xProcessed = false;

  // 遍历所有 chunks
  while (offset + 8 <= inputData_.size()) {
    uint32_t chunkId = (static_cast<uint32_t>(inputData_[offset]) |
                        (static_cast<uint32_t>(inputData_[offset + 1]) << 8) |
                        (static_cast<uint32_t>(inputData_[offset + 2]) << 16) |
                        (static_cast<uint32_t>(inputData_[offset + 3]) << 24));
    
    uint32_t chunkSize = (static_cast<uint32_t>(inputData_[offset + 4]) |
                          (static_cast<uint32_t>(inputData_[offset + 5]) << 8) |
                          (static_cast<uint32_t>(inputData_[offset + 6]) << 16) |
                          (static_cast<uint32_t>(inputData_[offset + 7]) << 24));
    
    size_t chunkTotalSize = 8 + chunkSize;
    if (chunkSize % 2 != 0) {
      chunkTotalSize++; // Account for padding
    }
    
    if (offset + chunkTotalSize > inputData_.size()) {
      break;
    }

    // 处理 VP8X chunk
    if (chunkId == 0x58385056) { // 'VP8X'
      if (!vp8xProcessed) {
        // 更新 VP8X chunk 的 XMP flag
        std::vector<uint8_t> updatedVP8X(inputData_.begin() + offset, 
                                          inputData_.begin() + offset + chunkTotalSize);
        // Set XMP flag (bit 2) in flags byte (at offset 8 in chunk)
        updatedVP8X[8] |= 0x04;
        out.write(reinterpret_cast<const char*>(updatedVP8X.data()), updatedVP8X.size());
        vp8xProcessed = true;
      }
      offset += chunkTotalSize;
      continue;
    }

    // 如果需要插入 VP8X chunk 且还没处理过
    if (needsVP8X && !vp8xProcessed) {
      out.write(reinterpret_cast<const char*>(vp8xChunk.data()), vp8xChunk.size());
      vp8xProcessed = true;
    }

    // 跳过旧的 XMP chunk
    if (chunkId == WEBP_FOURCC_XMP) {
      offset += chunkTotalSize;
      continue;
    }

    // 写入当前 chunk
    out.write(reinterpret_cast<const char*>(&inputData_[offset]), chunkTotalSize);
    offset += chunkTotalSize;
  }

  // 在文件末尾插入新的 XMP chunk
  if (!xmpInserted) {
    out.write(reinterpret_cast<const char*>(xmpChunk.data()), xmpChunk.size());
  }

  // 获取最终文件大小
  std::streampos finalPos = out.tellp();
  size_t totalSize = static_cast<size_t>(finalPos);

  // 更新 RIFF header 中的文件大小 (总大小 - 8)
  uint32_t fileSizeMinus8 = static_cast<uint32_t>(totalSize - 8);
  out.seekp(4);
  out.put(static_cast<char>(fileSizeMinus8 & 0xFF));
  out.put(static_cast<char>((fileSizeMinus8 >> 8) & 0xFF));
  out.put(static_cast<char>((fileSizeMinus8 >> 16) & 0xFF));
  out.put(static_cast<char>((fileSizeMinus8 >> 24) & 0xFF));

  return out.good();
}

} // namespace gimt

