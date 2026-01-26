//
// Test suite for PngAIGCReader and PngAIGCWriter - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_png_aigc_reader.h"
#include "gimt/gimt_png_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <gmock/gmock.h>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class PngAIGCReaderTest : public Test {
public:
  PngAIGCReader reader;
  ResourcePathFinder finder;
  AIGCInfo info;
};

// ============================================================================
// Behavior: Reader should handle file preparation correctly
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldFailToPreparePNGWhenFileDoesNotExist) {
  // Given a non-existent file path
  std::string nonExistentPath = "this_file_does_not_exist_at_all.png";
  
  // When attempting to prepare the reader
  bool result = reader.prepare(nonExistentPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(PngAIGCReaderTest, ReaderShouldSuccessfullyPrepareWhenPNGFileExists) {
  // Given an existing PNG file
  auto pngPath = finder.find("png_empty.png");
  
  // When preparing the reader
  bool result = reader.prepare(pngPath);
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(PngAIGCReaderTest, ReaderShouldHandleMultiplePrepareCallsGracefully) {
  // Given an existing PNG file
  auto pngPath = finder.find("png_empty.png");
  
  // When preparing the reader multiple times
  bool firstPrepare = reader.prepare(pngPath);
  bool secondPrepare = reader.prepare(pngPath);
  
  // Then both preparations should succeed
  ASSERT_THAT(firstPrepare, IsTrue());
  ASSERT_THAT(secondPrepare, IsTrue());
}

// ============================================================================
// Behavior: Reader should validate file format before processing
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldFailToReadAIGCInfoWithoutPreparation) {
  // Given a reader that has not been prepared
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(PngAIGCReaderTest, ReaderShouldRejectNonPNGFileFormat) {
  // Given a JPEG file (not a PNG)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_THAT(reader.prepare(jpegPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should handle PNG files without XMP metadata
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldReturnFalseForPNGWithoutXMPMetadata) {
  // Given a PNG file without XMP metadata
  auto pngPath = finder.find("png_empty.png");
  ASSERT_THAT(reader.prepare(pngPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail (no XMP data present)
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should extract AIGC info from PNG with XMP metadata
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldSuccessfullyExtractAIGCInfoFromPNGWithXMP) {
  // Given a PNG file containing XMP metadata with AIGC info
  auto pngPath = finder.find("png_with_xmp.png");
  ASSERT_THAT(reader.prepare(pngPath), IsTrue());
  
  // When reading AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then extraction should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(PngAIGCReaderTest, ReaderShouldExtractCorrectLabelFromXMP) {
  // Given a PNG file with XMP containing AIGC info
  auto pngPath = finder.find("png_with_xmp.png");
  reader.prepare(pngPath);
  
  // When reading AIGC info
  reader.readAIGCInfo(info);
  
  // Then the label should match expected value
  ASSERT_THAT(info.label, Eq("1"));
}

TEST_F(PngAIGCReaderTest, ReaderShouldExtractAllAIGCFieldsCorrectly) {
  // Given a PNG file with complete AIGC metadata
  auto pngPath = finder.find("png_with_xmp.png");
  reader.prepare(pngPath);
  
  // When reading AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then all fields should be extracted correctly
  ASSERT_THAT(result, IsTrue());
  
  AIGCInfo expected{
    .label = "1",
    .contentProducer = "ContentProducer",
    .produceID = "ProduceID",
    .reservedCode1 = "ReservedCode1",
    .contentPropagator = "ContentPropagator",
    .propagateID = "PropagateID",
    .reservedCode2 = "ReservedCode2"
  };
  
  ASSERT_THAT(info, Eq(expected));
}

// ============================================================================
// Behavior: Reader should support multiple reads on same file
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldAllowMultipleReadsOnSameFile) {
  // Given a prepared PNG file with XMP
  auto pngPath = finder.find("png_with_xmp.png");
  reader.prepare(pngPath);
  
  // When reading AIGC info multiple times
  AIGCInfo firstRead, secondRead;
  bool firstResult = reader.readAIGCInfo(firstRead);
  bool secondResult = reader.readAIGCInfo(secondRead);
  
  // Then both reads should succeed and return identical data
  ASSERT_THAT(firstResult, IsTrue());
  ASSERT_THAT(secondResult, IsTrue());
  ASSERT_THAT(firstRead, Eq(secondRead));
}

// ============================================================================
// Behavior: Reader should be resilient to different file states
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldSwitchBetweenDifferentFiles) {
  // Given two different PNG files
  auto emptyPng = finder.find("png_empty.png");
  auto xmpPng = finder.find("png_with_xmp.png");
  
  // When switching between files
  reader.prepare(emptyPng);
  bool emptyResult = reader.readAIGCInfo(info);
  
  reader.prepare(xmpPng);
  bool xmpResult = reader.readAIGCInfo(info);
  
  // Then each file should be processed correctly
  ASSERT_THAT(emptyResult, IsFalse()); // empty has no XMP
  ASSERT_THAT(xmpResult, IsTrue());    // xmp file has data
  ASSERT_THAT(info.label, Eq("1"));
}

// ============================================================================
// Behavior: Reader should handle edge cases gracefully
// ============================================================================

TEST_F(PngAIGCReaderTest, ReaderShouldHandleEmptyFilePath) {
  // Given an empty file path
  std::string emptyPath = "";
  
  // When attempting to prepare
  bool result = reader.prepare(emptyPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(PngAIGCReaderTest, ReaderShouldMaintainStateAfterFailedRead) {
  // Given a PNG without XMP
  auto emptyPng = finder.find("png_empty.png");
  reader.prepare(emptyPng);
  
  // When attempting a failed read followed by successful preparation
  bool failedRead = reader.readAIGCInfo(info);
  
  auto xmpPng = finder.find("png_with_xmp.png");
  reader.prepare(xmpPng);
  bool successRead = reader.readAIGCInfo(info);
  
  // Then the reader should recover and work correctly
  ASSERT_THAT(failedRead, IsFalse());
  ASSERT_THAT(successRead, IsTrue());
}
