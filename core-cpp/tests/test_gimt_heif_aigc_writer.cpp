//
// Test suite for HeifAIGCWriter - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_binary_reader.h"
#include "gimt/gimt_heif_aigc_reader.h"
#include "gimt/gimt_heif_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <fstream>
#include <gmock/gmock.h>
#include <vector>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class HeifAIGCWriterTest : public Test {
public:
  ResourcePathFinder finder;
  HeifAIGCWriter writer;
  HeifAIGCReader reader;
};

// ============================================================================
// Behavior: Writer should validate input files before preparation
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldFailToPrepareWhenInputFileDoesNotExist) {
  // Given a non-existent input file path
  std::string nonExistentPath = "non_existent_input.heif";
  std::string outputPath = "output.heif";
  
  // When attempting to prepare the writer
  bool result = writer.prepare(nonExistentPath, outputPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCWriterTest, WriterShouldRejectNonHeifFileFormat) {
  // Given a JPEG file (not a HEIF)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_FALSE(jpegPath.empty());
  
  // When attempting to prepare with a non-HEIF file
  bool result = writer.prepare(jpegPath, "output_non_heif.heif");
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCWriterTest, WriterShouldSuccessfullyPrepareWithValidHeif) {
  // Given a valid HEIF file
  auto heifPath = finder.find("heif_empty.heif");
  ASSERT_FALSE(heifPath.empty());
  
  // When preparing the writer
  bool result = writer.prepare(heifPath, "output_valid_heif.heif");
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

// ============================================================================
// Behavior: Writer should enforce proper workflow (prepare before write)
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldFailToWriteAIGCInfoWithoutPreparation) {
  // Given a writer that has not been prepared
  AIGCInfo info;
  
  // When attempting to write AIGC info
  bool result = writer.writeAIGCInfo(info);
  
  // Then writing should fail
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Writer should correctly embed AIGC metadata into HEIF
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldEmbedAIGCMetadataIntoHeif) {
  // Given a valid HEIF file and AIGC info
  auto heifPath = finder.find("heif_empty.heif");
  ASSERT_FALSE(heifPath.empty());
  
  const std::string outPath = "writer_output_with_aigc.heif";
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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

TEST_F(HeifAIGCWriterTest, WriterShouldCreateReadableOutputFile) {
  // Given a HEIF file with written AIGC metadata
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_readable.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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
// Behavior: Written metadata should be readable by HeifAIGCReader
// ============================================================================

TEST_F(HeifAIGCWriterTest, WrittenAIGCInfoShouldBeReadableByReader) {
  // Given a HEIF file with AIGC metadata written
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_roundtrip.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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
  
  // When reading back the AIGC info using HeifAIGCReader
  ASSERT_TRUE(reader.prepare(outPath));
  
  AIGCInfo actual;
  bool readResult = reader.readAIGCInfo(actual);
  
  // Then the read should succeed and data should match
  ASSERT_TRUE(readResult);
  ASSERT_THAT(actual, Eq(expected));
}

TEST_F(HeifAIGCWriterTest, WrittenMetadataShouldPreserveAllAIGCFields) {
  // Given AIGC info with all fields populated
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_all_fields.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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

TEST_F(HeifAIGCWriterTest, WriterShouldHandleSpecialCharactersInMetadata) {
  // Given AIGC info with special XML characters
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_special_chars.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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
// Behavior: Writer should maintain HEIF file structure
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldPreserveFtypBox) {
  // Given a HEIF file with AIGC metadata written
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_ftyp.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "P", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Prop",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading the output file header
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Then the ftyp box should be intact
  uint32_t boxSize = fileReader.readU32BE();
  ASSERT_GT(boxSize, 0u);
  
  uint32_t boxType = fileReader.readU32BE();
  ASSERT_EQ(boxType, HEIF_BOX_FTYP);
  
  uint32_t majorBrand = fileReader.readU32BE();
  // Should be a valid HEIF brand
  bool isValidBrand = (majorBrand == HEIF_BRAND_HEIC || 
                       majorBrand == HEIF_BRAND_MIF1 ||
                       majorBrand == HEIF_BRAND_HEVC ||
                       majorBrand == HEIF_BRAND_HEVX);
  ASSERT_TRUE(isValidBrand);
}

TEST_F(HeifAIGCWriterTest, WriterShouldCreateValidMetaBox) {
  // Given a HEIF file with AIGC metadata
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_meta_box.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "Producer", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Propagator",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When examining the meta box
  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());
  
  BinaryReader fileReader(outFile);
  
  // Skip ftyp box
  uint32_t ftypSize = fileReader.readU32BE();
  fileReader.skip(ftypSize - 4);
  
  // Read next box (should be meta or another top-level box)
  bool foundMeta = false;
  while (!fileReader.isEOF()) {
    uint32_t boxSize = fileReader.readU32BE();
    if (boxSize == 0) break;
    
    uint32_t boxType = fileReader.readU32BE();
    
    if (boxType == HEIF_BOX_META) {
      foundMeta = true;
      
      // Meta is a FullBox, should have version/flags
      uint8_t version = 0;
      fileReader.readBytes(&version, 1);
      ASSERT_EQ(version, 0); // Version should be 0
      
      break;
    }
    
    fileReader.skip(boxSize - 8);
  }
  
  // Then meta box should exist
  ASSERT_TRUE(foundMeta);
}

// ============================================================================
// Behavior: Writer should handle existing XMP metadata
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldOverwriteExistingXMPMetadata) {
  // Given a HEIF file that already has XMP metadata
  auto heifPath = finder.find("heif_with_xmp.heif");
  ASSERT_FALSE(heifPath.empty());
  
  const std::string outPath = "writer_output_overwrite_xmp.heif";
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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
// Behavior: Writer should handle edge cases gracefully
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldHandleEmptyAIGCFields) {
  // Given AIGC info with empty fields
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_empty_fields.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
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

TEST_F(HeifAIGCWriterTest, WriterShouldAllowMultipleWriteOperations) {
  // Given a HEIF file
  auto heifPath = finder.find("heif_empty.heif");
  
  // When writing to multiple output files
  const std::string outPath1 = "writer_output_multi_1.heif";
  const std::string outPath2 = "writer_output_multi_2.heif";
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  AIGCInfo info2{.label = "1", .contentProducer = "P2", .produceID = "PID2",
                 .reservedCode1 = "R1", .contentPropagator = "Prop2",
                 .propagateID = "PropID2", .reservedCode2 = "R2_2"};
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath1));
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath2));
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

TEST_F(HeifAIGCWriterTest, WriterShouldResetStateOnNewPreparation) {
  // Given a writer that has been used before
  auto heifPath1 = finder.find("heif_empty.heif");
  const std::string outPath1 = "writer_output_state_1.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath1, outPath1));
  
  AIGCInfo info1{.label = "1", .contentProducer = "P1", .produceID = "PID1",
                 .reservedCode1 = "R1", .contentPropagator = "Prop1",
                 .propagateID = "PropID1", .reservedCode2 = "R2_1"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info1));
  
  // When preparing with a different file
  auto heifPath2 = finder.find("heif_with_xmp.heif");
  const std::string outPath2 = "writer_output_state_2.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath2, outPath2));
  
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
// Behavior: Output file should be valid HEIF that can be read by reader
// ============================================================================

TEST_F(HeifAIGCWriterTest, OutputFileShouldBeValidHeifFormat) {
  // Given a HEIF file with AIGC metadata written
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_valid_format.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "Producer", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Propagator",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When attempting to prepare the reader with the output file
  bool readerPrepareResult = reader.prepare(outPath);
  
  // Then the reader should successfully prepare (file is valid HEIF)
  ASSERT_TRUE(readerPrepareResult);
}

TEST_F(HeifAIGCWriterTest, WriterShouldCreateXMPItemInMetaBox) {
  // Given a HEIF file with AIGC metadata written
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_xmp_item.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  AIGCInfo info{.label = "1", .contentProducer = "Producer", .produceID = "PID",
                .reservedCode1 = "R1", .contentPropagator = "Propagator",
                .propagateID = "PropID", .reservedCode2 = "R2"};
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading the AIGC info back
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  bool readResult = reader.readAIGCInfo(actual);
  
  // Then the XMP item should exist and be readable
  ASSERT_TRUE(readResult);
  ASSERT_THAT(actual, Eq(info));
}

// ============================================================================
// Behavior: Writer should handle long metadata strings
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterShouldHandleLongMetadataStrings) {
  // Given AIGC info with long strings
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_long_strings.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  std::string longString(200, 'A');
  AIGCInfo expected{
    .label = "1",
    .contentProducer = longString,
    .produceID = longString,
    .reservedCode1 = longString,
    .contentPropagator = longString,
    .propagateID = longString,
    .reservedCode2 = longString
  };
  
  // When writing long AIGC info
  ASSERT_TRUE(writer.writeAIGCInfo(expected));
  
  // Then it should be readable
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // And all fields should match
  ASSERT_THAT(actual, Eq(expected));
}

// ============================================================================
// Behavior: Writer should produce files verifiable with exiftool
// ============================================================================

TEST_F(HeifAIGCWriterTest, WriterOutputShouldContainValidXMPStructure) {
  // Given a HEIF file with AIGC metadata written
  auto heifPath = finder.find("heif_empty.heif");
  const std::string outPath = "writer_output_xmp_structure.heif";
  
  ASSERT_TRUE(writer.prepare(heifPath, outPath));
  
  AIGCInfo info{
    .label = "1",
    .contentProducer = "TestProducer",
    .produceID = "TestPID",
    .reservedCode1 = "TestR1",
    .contentPropagator = "TestPropagator",
    .propagateID = "TestPropID",
    .reservedCode2 = "TestR2"
  };
  
  ASSERT_TRUE(writer.writeAIGCInfo(info));
  
  // When reading back with reader
  ASSERT_TRUE(reader.prepare(outPath));
  AIGCInfo actual;
  ASSERT_TRUE(reader.readAIGCInfo(actual));
  
  // Then the data should match (indicating valid XMP structure)
  ASSERT_THAT(actual, Eq(info));
  
  // Note: For full verification, run: exiftool -v2 writer_output_xmp_structure.heif
  // This will show the XMP metadata in detail
}






