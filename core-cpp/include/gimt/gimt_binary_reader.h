//
// Created by user on 1/25/26.
//

#ifndef GIMT_BINARY_READER_H
#define GIMT_BINARY_READER_H
#pragma once
#include <cstdint>

namespace gimt {
class BinaryReader {
private:
  std::istream& stream;

public:
  explicit BinaryReader(std::istream& is) : stream(is) {}

  // 返回真正读取到的数量
  size_t readBytes(uint8_t* buffer, size_t size) {
    if (size == 0) return 0;
    stream.read(reinterpret_cast<char*>(buffer), size);
    return static_cast<size_t>(stream.gcount());
  }

  void skip(size_t bytes) {
    stream.seekg(bytes, std::ios::cur);
  }

  std::streampos tell() {
    return stream.tellg();
  }

  uint16_t readU16BE() {
    uint8_t buf[2];
    // 只有完整读到 2 字节才转换，否则返回 0
    if (readBytes(buf, 2) != 2) return 0;
    return (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
  }

  uint32_t readU32BE() {
    uint8_t buf[4];
    // 只有完整读到 4 字节才转换
    if (readBytes(buf, 4) != 4) return 0;
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
           buf[3];
  }

  bool isEOF() {
    return stream.peek() == EOF;
  }
};
}



#endif //GIMT_BINARY_READER_H
