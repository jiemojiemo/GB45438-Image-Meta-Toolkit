//
// Implementation of PngAIGCWriter
//

#include "gimt/gimt_png_aigc_writer.h"
#include "gimt/gimt_xml_utils.h"
#include "gimt/gimt_patter_matcher.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

namespace gimt {

// CRC32 查找表（PNG 标准）
static const uint32_t crc_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

uint32_t PngAIGCWriter::calculateCRC32(const uint8_t* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc = crc_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFF;
}

void PngAIGCWriter::writeU32BE(std::vector<uint8_t>& buffer, uint32_t value) {
  buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
  buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
  buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  buffer.push_back(static_cast<uint8_t>(value & 0xFF));
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
  writeU32BE(fullChunk, dataLength);
  
  // Type (4 bytes): "iTXt"
  fullChunk.push_back('i');
  fullChunk.push_back('T');
  fullChunk.push_back('X');
  fullChunk.push_back('t');
  
  // Data
  fullChunk.insert(fullChunk.end(), chunk.begin(), chunk.end());
  
  // CRC (4 bytes) - 对 Type + Data 计算
  uint32_t crc = calculateCRC32(fullChunk.data() + 4, 4 + dataLength);
  writeU32BE(fullChunk, crc);
  
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
