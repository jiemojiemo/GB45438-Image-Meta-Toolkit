//
// Test suite for WebpAIGCWriter - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_binary_reader.h"
#include "gimt/gimt_patter_matcher.h"
#include "gimt/gimt_webp_aigc_reader.h"
#include "gimt/gimt_webp_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <fstream>
#include <gmock/gmock.h>
#include <vector>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class WebpAIGCWriterTest : public Test {
public:
  ResourcePathFinder finder;
  WebpAIGCWriter writer;
  WebpAIGCReader reader;
};

// ============================================================================
// Behavior: Writer should validate input files before preparation
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldFailToPreparWhenInputFileDoesNotExist) {
  // Given a non-existent input file path
  std::string nonExistentPath = "non_existent_input.webp";
  std::string outputPath = "output.webp";
  
  // When attempting to prepare the writer
  bool result = writer.prepare(nonExistentPath, outputPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCWriterTest, WriterShouldRejectNonWebPFileFormat) {
  // Given a JPEG file (not a WebP)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_FALSE(jpegPath.empty());
  
  // When attempting to prepare with a non-WebP file
  bool result = writer.prepare(jpegPath, "output_non_webp.webp");
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCWriterTest, WriterShouldSuccessfullyPrepareWithValidWebP) {
  // Given a valid WebP file
  auto webpPath = finder.find("webp_empty.webp");
  ASSERT_FALSE(webpPath.empty());
  
  // When preparing the writer
  bool result = writer.prepare(webpPath, "output_valid_webp.webp");
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

// ============================================================================
// Behavior: Writer should enforce proper workflow (prepare before write)
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldFailToWriteAIGCInfoWithoutPreparation) {
  // Given a writer that has not been prepared
  AIGCInfo info;
  
  // When attempting to write AIGC info
  bool result = writer.writeAIGCInfo(info);
  
  // Then writing should fail
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Writer should correctly embed AIGC metadata into WebP
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldEmbedAIGCMetadataIntoWebP) {
  // Given a valid WebP file and AIGC info
  auto webpPath = finder.find("webp_empty.webp");
  ASSERT_FALSE(webpPath.empty());
  
  const std::string outPath = "writer_output_with_aigc.webp";
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo expected{
    .label = "1",
    .contentProducer = "TestProducer",
    .produceID = "TestProduceID",
    .reservedCode1 = "Reserved1",
    .contentPropagator = "TestPropagator",
    .propagateID = "TestPropagateID",
    .reservedCode2 = "Reserved2"
  };
  
  // When writing AIGC info
  bool writeResult = writer.writeAIGCInfo(expected);
  
  // Then writing should succeed
  ASSERT_TRUE(writeResult);
}

TEST_F(WebpAIGCWriterTest, WriterShouldCreateReadableOutputFile) {
  // Given a WebP file with written AIGC metadata
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_readable.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{
    .label = "1",
    .contentProducer = "Producer",
    .produceID = "PID",
    .reservedCode1 = "R1",
    .contentPropagator = "Propagator",
    .propagateID = "PropID",
    .reservedCode2 = "R2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When attempting to open the output file
  std::ifstream outFile(outPath, std::ios::binary);
  
  // Then the file should exist and be readable
  ASSERT_TRUE(outFile.is_open());
  ASSERT_TRUE(outFile.good());
}

// ============================================================================
// Behavior: Written metadata should be readable by WebpAIGCReader
// ============================================================================

TEST_F(WebpAIGCWriterTest, WrittenAIGCInfoShouldBeReadableByReader) {
  // Given a WebP file with AIGC metadata written
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_roundtrip.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo expected{
    .label = "1",
    .contentProducer = "ContentProducer",
    .produceID = "ProduceID",
    .reservedCode1 = "ReservedCode1",
    .contentPropagator = "ContentPropagator",
    .propagateID = "PropagateID",
    .reservedCode2 = "ReservedCode2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(expected));
  
  // When reading back the AIGC info using WebpAIGCReader
  ASSERT_TRUE(reader.prepare(outPath));
  
  AIGCInfo actual;
  bool readResult = reader.readAIGCInfo(actual);
  
  // Then the read should succeed and data should match
  ASSERT_TRUE(readResult);
  ASSERT_THAT(actual, Eq(expected));
}

TEST_F(WebpAIGCWriterTest, WrittenMetadataShouldPreserveAllAIGCFields) {
  // Given AIGC info with all fields populated
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_all_fields.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo expected{
    .label = "1",
    .contentProducer = "Producer123",
    .produceID = "PID456",
    .reservedCode1 = "RC1_789",
    .contentPropagator = "Propagator_ABC",
    .propagateID = "PropID_DEF",
    .reservedCode2 = "RC2_GHI"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(expected));
  
  // When reading back all fields
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // Then all fields should be preserved exactly
  ASSERT_THAT(actual.label, Eq(expected.label));
  ASSERT_THAT(actual.contentProducer, Eq(expected.contentProducer));
  ASSERT_THAT(actual.produceID, Eq(expected.produceID));
  ASSERT_THAT(actual.reservedCode1, Eq(expected.reservedCode1));
  ASSERT_THAT(actual.contentPropagator, Eq(expected.contentPropagator));
  ASSERT_THAT(actual.propagateID, Eq(expected.propagateID));
  ASSERT_THAT(actual.reservedCode2, Eq(expected.reservedCode2));
}

// ============================================================================
// Behavior: Writer should handle special characters in metadata
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldHandleSpecialCharactersInMetadata) {
  // Given AIGC info with special XML characters
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_special_chars.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo expected{
    .label = "1",
    .contentProducer = "Producer<>&",
    .produceID = "ID with spaces & symbols",
    .reservedCode1 = "Reserved<1>",
    .contentPropagator = "Propagator",
    .propagateID = "PropID",
    .reservedCode2 = "R&2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(expected));
  
  // When reading back the data
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // Then special characters should be preserved
  ASSERT_THAT(actual, Eq(expected));
}

// ============================================================================
// Behavior: Writer should insert XMP chunk in correct position
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldInsertXMPChunkAfterImageData) {
  // Given a WebP file
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_chunk_position.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{
    .label = "1",
    .contentProducer = "Producer",
    .produceID = "PID",
    .reservedCode1 = "R1",
    .contentPropagator = "Propagator",
    .propagateID = "PropID",
    .reservedCode2 = "R2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When examining the output file structure
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Skip RIFF header (12 bytes)
  fileReader.skip(12);
  
  // Read chunks to verify XMP exists
  bool foundXMP = false;
  bool foundImageData = false;
  
  while (!fileReader.isEOF()) {
    uint32_t chunkId = fileReader.readU32LE();
    if (chunkId == 0) break;
    
    uint32_t chunkSize = fileReader.readU32LE();
    if (chunkSize == 0 && fileReader.isEOF()) break;
    
    // Check for image data chunks (VP8, VP8L, VP8X)
    if (chunkId == 0x20385056 || chunkId == 0x4C385056 || chunkId == 0x58385056) {
      foundImageData = true;
    }
    
    if (chunkId == WEBP_FOURCC_XMP) {
      foundXMP = true;
    }
    
    // Skip chunk data and padding
    fileReader.skip(chunkSize);
    if (chunkSize % 2 != 0) {
      fileReader.skip(1);
    }
  }
  
  // Then XMP chunk should be found
  ASSERT_TRUE(foundXMP);
  ASSERT_TRUE(foundImageData);
}

TEST_F(WebpAIGCWriterTest, WriterShouldCreateValidXMPChunk) {
  // Given a WebP file with AIGC metadata
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_xmp_validation.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{
    .label = "1",
    .contentProducer = "Producer",
    .produceID = "PID",
    .reservedCode1 = "R1",
    .contentPropagator = "Propagator",
    .propagateID = "PropID",
    .reservedCode2 = "R2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When examining the XMP chunk content
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Skip RIFF header
  fileReader.skip(12);
  
  // Find XMP chunk
  while (!fileReader.isEOF()) {
    uint32_t chunkId = fileReader.readU32LE();
    if (chunkId == 0) break;
    
    uint32_t chunkSize = fileReader.readU32LE();
    if (chunkSize == 0 && fileReader.isEOF()) break;
    
    if (chunkId == WEBP_FOURCC_XMP) {
      // Read chunk data
      std::vector<uint8_t> chunkData(chunkSize);
      ASSERT_EQ(fileReader.readBytes(chunkData.data(), chunkSize), chunkSize);
      
      // Then the XMP should contain valid XML
      std::string xmpStr(reinterpret_cast<const char*>(chunkData.data()), chunkSize);
      ASSERT_TRUE(xmpStr.find("<x:xmpmeta") != std::string::npos);
      ASSERT_TRUE(xmpStr.find("</x:xmpmeta>") != std::string::npos);
      
      break;
    }
    
    // Skip chunk data and padding
    fileReader.skip(chunkSize);
    if (chunkSize % 2 != 0) {
      fileReader.skip(1);
    }
  }
}

// ============================================================================
// Behavior: Writer should handle existing XMP metadata
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldOverwriteExistingXMPMetadata) {
  // Given a WebP file that already has XMP metadata
  auto webpPath = finder.find("webp_with_xmp.webp");
  ASSERT_FALSE(webpPath.empty());
  
  const std::string outPath = "writer_output_overwrite_xmp.webp";
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo newInfo{
    .label = "1",
    .contentProducer = "NewProducer",
    .produceID = "NewPID",
    .reservedCode1 = "NewR1",
    .contentPropagator = "NewPropagator",
    .propagateID = "NewPropID",
    .reservedCode2 = "NewR2"
  };
  
  // When writing new AIGC info
  ASSERT_TRUE(writer.writeAIGCInfo(newInfo));
  
  // Then the new info should be readable
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // And it should match the new info (not the old one)
  ASSERT_THAT(actual, Eq(newInfo));
  ASSERT_THAT(actual.contentProducer, Eq("NewProducer"));
}

// ============================================================================
// Behavior: Writer should preserve WebP file structure
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldPreserveRIFFHeader) {
  // Given a WebP file with AIGC metadata written
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_riff_header.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "P", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Prop",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading the output file header
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Then the RIFF header should be intact
  uint32_t riffTag = fileReader.readU32LE();
  ASSERT_EQ(riffTag, WEBP_FOURCC_RIFF);
  
  uint32_t fileSize = fileReader.readU32LE();
  ASSERT_GT(fileSize, 0u);
  
  uint32_t webpTag = fileReader.readU32LE();
  ASSERT_EQ(webpTag, WEBP_FOURCC_WEBP);
}

TEST_F(WebpAIGCWriterTest, WriterShouldUpdateFileSizeInRIFFHeader) {
  // Given a WebP file with AIGC metadata written
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_file_size.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "Producer", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Propagator",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When checking the file size
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  // Get actual file size
  outFile.seekg(0, std::ios::end);
  size_t actualFileSize = outFile.tellg();
  outFile.seekg(0, std::ios::beg);
  
  BinaryReader fileReader(outFile);
  fileReader.skip(4); // Skip RIFF tag
  
  uint32_t declaredSize = fileReader.readU32LE();
  
  // Then the declared size should match actual size - 8
  ASSERT_EQ(declaredSize, actualFileSize - 8);
}

// ============================================================================
// Behavior: Writer should handle VP8X chunk correctly
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldSetXMPFlagInVP8XWhenPresent) {
  // Given a WebP file that might have VP8X
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_vp8x_flag.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "Producer", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Propagator",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When examining VP8X chunk
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  fileReader.skip(12); // Skip RIFF header
  
  uint32_t firstChunkId = fileReader.readU32LE();
  
  // If first chunk is VP8X
  if (firstChunkId == 0x58385056) { // 'VP8X'
    uint32_t chunkSize = fileReader.readU32LE();
    ASSERT_EQ(chunkSize, 10u); // VP8X payload is always 10 bytes
    
    uint8_t flags = 0;
    fileReader.readBytes(&flags, 1);
    
    // Then XMP flag (bit 2) should be set
    ASSERT_TRUE((flags & 0x04) != 0);
  }
}

// ============================================================================
// Behavior: Writer should handle edge cases gracefully
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldHandleEmptyAIGCFields) {
  // Given AIGC info with empty fields
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_empty_fields.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  AIGCInfo expected{
    .label = "",
    .contentProducer = "",
    .produceID = "",
    .reservedCode1 = "",
    .contentPropagator = "",
    .propagateID = "",
    .reservedCode2 = ""
  };
  
  // When writing empty AIGC info
  ASSERT_TRUE(writer.writeAIGCInfo(expected));
  
  // Then it should be readable
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // And all fields should be empty
  ASSERT_THAT(actual, Eq(expected));
}

TEST_F(WebpAIGCWriterTest, WriterShouldAllowMultipleWriteOperations) {
  // Given a WebP file
  auto webpPath = finder.find("webp_empty.webp");
  
  // When writing to multiple output files
  const std::string outPath1 = "writer_output_multi_1.webp";
  const std::string outPath2 = "writer_output_multi_2.webp";
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  AIGCInfo info2{.label = "1", .contentProducer = "P2", .produceID = "PID2",
                 .reservedCode1 = "R1", .contentPropagator = "Prop2",
                 .propagateID = "PropID2", .reservedCode2 = "R2_2"};
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath1));
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath2));
  ASSERT_TRUE(writer.writeAIGCInfo(info2));
  
  // Then both files should contain correct data
  ASSERT_TRUE(reader.prepare(outPath1));
  AIGCInfo actual1;
  ASSERT_TRUE(reader.readAIGCInfo(actual1));
  ASSERT_THAT(actual1, Eq(info1));
  
  ASSERT_TRUE(reader.prepare(outPath2));
  AIGCInfo actual2;
  ASSERT_TRUE(reader.readAIGCInfo(actual2));
  ASSERT_THAT(actual2, Eq(info2));
}

// ============================================================================
// Behavior: Writer should maintain state correctly
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldResetStateOnNewPreparation) {
  // Given a writer that has been used before
  auto webpPath1 = finder.find("webp_empty.webp");
  const std::string outPath1 = "writer_output_state_1.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath1, outPath1));
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  // When preparing with a different file
  auto webpPath2 = finder.find("webp_with_xmp.webp");
  const std::string outPath2 = "writer_output_state_2.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath2, outPath2));
  
  AIGCInfo info2{.label = "1", .contentProducer = "P2", .produceID = "PID2",
                 .reservedCode1 = "R1", .contentPropagator = "Prop2",
                 .propagateID = "PropID2", .reservedCode2 = "R2_2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info2));
  
  // Then the second file should contain the correct data
  ASSERT_TRUE(reader.prepare(outPath2));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  ASSERT_THAT(actual, Eq(info2));
}

// ============================================================================
// Behavior: Writer should handle chunk padding correctly
// ============================================================================

TEST_F(WebpAIGCWriterTest, WriterShouldAddPaddingForOddSizedXMPChunk) {
  // Given AIGC info that will result in odd-sized XMP chunk
  auto webpPath = finder.find("webp_empty.webp");
  const std::string outPath = "writer_output_padding.webp";
  
  ASSERT_TRUE(writer.prepare(webpPath, outPath));
  
  // Use a short string to potentially create odd-sized chunk
  AIGCInfo info{.label = "1", .contentProducer = "P", .produceID = "P",
                .reservedCode1 = "R", .contentPropagator = "P",
                .propagateID = "P", .reservedCode2 = "R"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading back the data
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  
  // Then it should be readable without errors
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  ASSERT_THAT(actual, Eq(info));
}

