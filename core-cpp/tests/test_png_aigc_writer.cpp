//
// Test suite for PngAIGCWriter - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_binary_reader.h"
#include "gimt/gimt_patter_matcher.h"
#include "gimt/gimt_png_aigc_reader.h"
#include "gimt/gimt_png_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <fstream>
#include <gmock/gmock.h>
#include <vector>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class PngAIGCWriterTest : public Test {
public:
  ResourcePathFinder finder;
  PngAIGCWriter writer;
  PngAIGCReader reader;
};

// ============================================================================
// Behavior: Writer should validate input files before preparation
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldFailToPreparePNGWhenInputFileDoesNotExist) {
  // Given a non-existent input file path
  std::string nonExistentPath = "non_existent_input.png";
  std::string outputPath = "output.png";
  
  // When attempting to prepare the writer
  bool result = writer.prepare(nonExistentPath, outputPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(PngAIGCWriterTest, WriterShouldRejectNonPNGFileFormat) {
  // Given a JPEG file (not a PNG)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_FALSE(jpegPath.empty());
  
  // When attempting to prepare with a non-PNG file
  bool result = writer.prepare(jpegPath, "output_non_png.png");
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(PngAIGCWriterTest, WriterShouldSuccessfullyPrepareWithValidPNG) {
  // Given a valid PNG file
  auto pngPath = finder.find("png_empty.png");
  ASSERT_FALSE(pngPath.empty());
  
  // When preparing the writer
  bool result = writer.prepare(pngPath, "output_valid_png.png");
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

// ============================================================================
// Behavior: Writer should enforce proper workflow (prepare before write)
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldFailToWriteAIGCInfoWithoutPreparation) {
  // Given a writer that has not been prepared
  AIGCInfo info;
  
  // When attempting to write AIGC info
  bool result = writer.writeAIGCInfo(info);
  
  // Then writing should fail
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Writer should correctly embed AIGC metadata into PNG
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldEmbedAIGCMetadataIntoPNG) {
  // Given a valid PNG file and AIGC info
  auto pngPath = finder.find("png_empty.png");
  ASSERT_FALSE(pngPath.empty());
  
  const std::string outPath = "writer_output_with_aigc.png";
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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

TEST_F(PngAIGCWriterTest, WriterShouldCreateReadableOutputFile) {
  // Given a PNG file with written AIGC metadata
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_readable.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
// Behavior: Written metadata should be readable by PngAIGCReader
// ============================================================================

TEST_F(PngAIGCWriterTest, WrittenAIGCInfoShouldBeReadableByReader) {
  // Given a PNG file with AIGC metadata written
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_roundtrip.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
  
  // When reading back the AIGC info using PngAIGCReader
  ASSERT_TRUE(reader.prepare(outPath));
  
  AIGCInfo actual;
  bool readResult = reader.readAIGCInfo(actual);
  
  // Then the read should succeed and data should match
  ASSERT_TRUE(readResult);
  ASSERT_THAT(actual, Eq(expected));
}

TEST_F(PngAIGCWriterTest, WrittenMetadataShouldPreserveAllAIGCFields) {
  // Given AIGC info with all fields populated
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_all_fields.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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

TEST_F(PngAIGCWriterTest, WriterShouldHandleSpecialCharactersInMetadata) {
  // Given AIGC info with special XML characters (excluding quotes which require JSON escaping)
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_special_chars.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
// Behavior: Writer should insert iTXt chunk in correct position
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldInsertITXtChunkBeforeIDAT) {
  // Given a PNG file
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_chunk_position.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
  
  // Skip PNG signature
  uint8_t signature[8];
  ASSERT_EQ(fileReader.readBytes(signature, 8), 8u);
  
  // Read chunks until we find iTXt or IDAT
  bool foundITXt = false;
  bool foundIDAT = false;
  
  while (!fileReader.isEOF()) {
    uint32_t chunkLength = fileReader.readU32BE();
    uint8_t chunkType[4];
    
    if (fileReader.readBytes(chunkType, 4) != 4) {
      break;
    }
    
    if (PatternMatcher::match(chunkType, 4, getPngChunkITXt())) {
      foundITXt = true;
      ASSERT_FALSE(foundIDAT) << "iTXt should appear before IDAT";
    }
    
    if (chunkType[0] == 'I' && chunkType[1] == 'D' && 
        chunkType[2] == 'A' && chunkType[3] == 'T') {
      foundIDAT = true;
    }
    
    // Skip chunk data and CRC
    fileReader.skip(chunkLength + 4);
  }
  
  // Then iTXt chunk should be found before IDAT
  ASSERT_TRUE(foundITXt);
  ASSERT_TRUE(foundIDAT);
}

TEST_F(PngAIGCWriterTest, WriterShouldCreateValidITXtChunkWithXMPKeyword) {
  // Given a PNG file with AIGC metadata
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_itxt_validation.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
  
  // When examining the iTXt chunk content
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Skip PNG signature
  uint8_t signature[8];
  fileReader.readBytes(signature, 8);
  
  // Find iTXt chunk
  while (!fileReader.isEOF()) {
    uint32_t chunkLength = fileReader.readU32BE();
    uint8_t chunkType[4];
    
    if (fileReader.readBytes(chunkType, 4) != 4) {
      break;
    }
    
    if (PatternMatcher::match(chunkType, 4, getPngChunkITXt())) {
      // Read chunk data
      std::vector<uint8_t> chunkData(chunkLength);
      ASSERT_EQ(fileReader.readBytes(chunkData.data(), chunkLength), chunkLength);
      
      // Then the keyword should be "XML:com.adobe.xmp"
      const std::string expectedKeyword = PNG_XMP_KEYWORD;
      ASSERT_TRUE(PatternMatcher::matchString(chunkData.data(), chunkLength, expectedKeyword));
      
      // Verify null terminator after keyword
      ASSERT_EQ(chunkData[PNG_XMP_KEYWORD_LEN], 0);
      
      // Verify compression flag is 0 (uncompressed)
      ASSERT_EQ(chunkData[PNG_XMP_KEYWORD_LEN + 1], 0);
      
      break;
    }
    
    // Skip chunk data and CRC
    fileReader.skip(chunkLength + 4);
  }
}

// ============================================================================
// Behavior: Writer should handle existing XMP metadata
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldOverwriteExistingXMPMetadata) {
  // Given a PNG file that already has XMP metadata
  auto pngPath = finder.find("png_with_xmp.png");
  ASSERT_FALSE(pngPath.empty());
  
  const std::string outPath = "writer_output_overwrite_xmp.png";
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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
// Behavior: Writer should preserve PNG image data integrity
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldPreservePNGSignature) {
  // Given a PNG file with AIGC metadata written
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_signature.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "P", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Prop",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading the output file signature
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  uint8_t signature[8];
  outFile.read(reinterpret_cast<char*>(signature), 8);
  
  // Then the PNG signature should be intact
  auto expectedSig = getPngSignature();
  ASSERT_TRUE(PatternMatcher::match(signature, 8, expectedSig));
}

// ============================================================================
// Behavior: Writer should handle edge cases gracefully
// ============================================================================

TEST_F(PngAIGCWriterTest, WriterShouldHandleEmptyAIGCFields) {
  // Given AIGC info with empty fields
  auto pngPath = finder.find("png_empty.png");
  const std::string outPath = "writer_output_empty_fields.png";
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath));
  
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

TEST_F(PngAIGCWriterTest, WriterShouldAllowMultipleWriteOperations) {
  // Given a PNG file
  auto pngPath = finder.find("png_empty.png");
  
  // When writing to multiple output files
  const std::string outPath1 = "writer_output_multi_1.png";
  const std::string outPath2 = "writer_output_multi_2.png";
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  AIGCInfo info2{.label = "1", .contentProducer = "P2", .produceID = "PID2",
                 .reservedCode1 = "R1", .contentPropagator = "Prop2",
                 .propagateID = "PropID2", .reservedCode2 = "R2_2"};
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath1));
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  ASSERT_TRUE(writer.prepare(pngPath, outPath2));
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

TEST_F(PngAIGCWriterTest, WriterShouldResetStateOnNewPreparation) {
  // Given a writer that has been used before
  auto pngPath1 = finder.find("png_empty.png");
  const std::string outPath1 = "writer_output_state_1.png";
  
  ASSERT_TRUE(writer.prepare(pngPath1, outPath1));
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  // When preparing with a different file
  auto pngPath2 = finder.find("png_with_xmp.png");
  const std::string outPath2 = "writer_output_state_2.png";
  
  ASSERT_TRUE(writer.prepare(pngPath2, outPath2));
  
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
