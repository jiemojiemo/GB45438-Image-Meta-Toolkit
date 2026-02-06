//
// HEIF/HEIC AIGC Writer - Writes AIGC metadata to HEIF/HEIC files via XMP
//

#ifndef GIMT_HEIF_AIGC_WRITER_H
#define GIMT_HEIF_AIGC_WRITER_H

#include "gimt_def.h"
#include "gimt_aigc_writer.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gimt {

class HeifAIGCWriter : public IAIGCWriter {
public:
  HeifAIGCWriter() = default;

  // 准备写入：读取输入 HEIF/HEIC 并记录输出路径
  bool prepare(const std::string &inputFilepath, const std::string &outputFilepath) override;

  // 将 AIGCInfo 写入输出 HEIF/HEIC 的 XMP item 中
  bool writeAIGCInfo(const AIGCInfo &info) override;

  // 获取支持的图像格式
  ImageFormat getFormat() const override { return ImageFormat::HEIF; }

private:
  std::vector<uint8_t> inputData_;
  std::string outputPath_;

  // Box header structure
  struct BoxHeader {
    uint64_t size;
    uint32_t type;
    uint64_t headerSize;
  };

  // Item location structure
  struct ItemLocation {
    uint32_t itemID;
    uint64_t baseOffset;
    uint64_t extentOffset;
    uint64_t extentLength;
  };

  // Parse structure to track box positions
  struct BoxPosition {
    size_t offset;
    uint64_t size;
    uint32_t type;
  };

  // Read box header from data
  bool readBoxHeader(size_t offset, BoxHeader& header) const;
  
  // Find XMP item ID in iinf box
  uint32_t findXmpItemID(size_t metaOffset, size_t metaSize) const;
  
  // Find the highest existing item ID
  uint32_t findMaxItemID(size_t metaOffset, size_t metaSize) const;
  
  // Find item location in iloc box
  bool findItemLocation(size_t metaOffset, size_t metaSize, uint32_t itemID, ItemLocation& loc) const;
  
  // Build infe entry for XMP
  std::vector<uint8_t> buildXmpInfeEntry(uint32_t itemID) const;
  
  // Build iloc entry for XMP
  std::vector<uint8_t> buildIlocEntry(uint32_t itemID, uint64_t offset, uint64_t length, 
                                       uint8_t offsetSize, uint8_t lengthSize, uint8_t baseOffsetSize) const;
};

} // namespace gimt

#endif // GIMT_HEIF_AIGC_WRITER_H

