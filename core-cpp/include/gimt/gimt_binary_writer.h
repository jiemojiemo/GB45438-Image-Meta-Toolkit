//
// Binary Writer Utility
//

#ifndef GIMT_BINARY_WRITER_H
#define GIMT_BINARY_WRITER_H
#pragma once
#include <cstdint>
#include <vector>

namespace gimt {
class BinaryWriter {
public:
  // Write uint32 in big-endian format to a vector
  static void writeU32BE(std::vector<uint8_t>& buffer, uint32_t value) {
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
  }

  // Write uint32 in little-endian format to a vector
  static void writeU32LE(std::vector<uint8_t>& buffer, uint32_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
  }

  // Write uint16 in big-endian format to a vector
  static void writeU16BE(std::vector<uint8_t>& buffer, uint16_t value) {
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
  }

  // Write uint16 in little-endian format to a vector
  static void writeU16LE(std::vector<uint8_t>& buffer, uint16_t value) {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  }
};
}

#endif //GIMT_BINARY_WRITER_H

