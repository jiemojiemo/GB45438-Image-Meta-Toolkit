//
// Test suite for WebpAIGCReader - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_webp_aigc_reader.h"
#include "gimt_testing_resource_finder.h"
#include <gmock/gmock.h>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class WebpAIGCReaderTest : public Test {
public:
  WebpAIGCReader reader;
  ResourcePathFinder finder;
  AIGCInfo info;
};

// ============================================================================
// Behavior: Reader should handle file preparation correctly
// ============================================================================

TEST_F(WebpAIGCReaderTest, ReaderShouldFailToPreparWebPWhenFileDoesNotExist) {
  // Given a non-existent file path
  std::string nonExistentPath = "this_file_does_not_exist_at_all.webp";
  
  // When attempting to prepare the reader
  bool result = reader.prepare(nonExistentPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldSuccessfullyPrepareWhenWebPFileExists) {
  // Given an existing WebP file
  auto webpPath = finder.find("webp_empty.webp");
  
  // When preparing the reader
  bool result = reader.prepare(webpPath);
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldHandleMultiplePrepareCallsGracefully) {
  // Given an existing WebP file
  auto webpPath = finder.find("webp_empty.webp");
  
  // When preparing the reader multiple times
  bool firstPrepare = reader.prepare(webpPath);
  bool secondPrepare = reader.prepare(webpPath);
  
  // Then both preparations should succeed
  ASSERT_THAT(firstPrepare, IsTrue());
  ASSERT_THAT(secondPrepare, IsTrue());
}

// ============================================================================
// Behavior: Reader should validate file format before processing
// ============================================================================

TEST_F(WebpAIGCReaderTest, ReaderShouldFailToReadAIGCInfoWithoutPreparation) {
  // Given a reader that has not been prepared
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldRejectNonWebPFileFormat) {
  // Given a JPEG file (not a WebP)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_THAT(reader.prepare(jpegPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldRejectPNGFileFormat) {
  // Given a PNG file (not a WebP)
  auto pngPath = finder.find("png_empty.png");
  ASSERT_THAT(reader.prepare(pngPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should handle WebP files without XMP metadata
// ============================================================================

TEST_F(WebpAIGCReaderTest, ReaderShouldReturnFalseForWebPWithoutXMPMetadata) {
  // Given a WebP file without XMP metadata
  auto webpPath = finder.find("webp_empty.webp");
  ASSERT_THAT(reader.prepare(webpPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail (no XMP data present)
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should extract AIGC info from WebP with XMP metadata
// ============================================================================

TEST_F(WebpAIGCReaderTest, ReaderShouldSuccessfullyExtractAIGCInfoFromWebPWithXMP) {
  // Given a WebP file containing XMP metadata with AIGC info
  auto webpPath = finder.find("webp_with_xmp.webp");
  ASSERT_THAT(reader.prepare(webpPath), IsTrue());
  
  // When reading AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then extraction should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldExtractCorrectLabelFromXMP) {
  // Given a WebP file with XMP containing AIGC info
  auto webpPath = finder.find("webp_with_xmp.webp");
  reader.prepare(webpPath);
  
  // When reading AIGC info
  reader.readAIGCInfo(info);
  
  // Then the label should match expected value
  ASSERT_THAT(info.label, Eq("1"));
}

TEST_F(WebpAIGCReaderTest, ReaderShouldExtractAllAIGCFieldsCorrectly) {
  // Given a WebP file with complete AIGC metadata
  auto webpPath = finder.find("webp_with_xmp.webp");
  reader.prepare(webpPath);
  
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

TEST_F(WebpAIGCReaderTest, ReaderShouldAllowMultipleReadsOnSameFile) {
  // Given a prepared WebP file with XMP
  auto webpPath = finder.find("webp_with_xmp.webp");
  reader.prepare(webpPath);
  
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

TEST_F(WebpAIGCReaderTest, ReaderShouldSwitchBetweenDifferentFiles) {
  // Given two different WebP files
  auto emptyWebp = finder.find("webp_empty.webp");
  auto xmpWebp = finder.find("webp_with_xmp.webp");
  
  // When switching between files
  reader.prepare(emptyWebp);
  bool emptyResult = reader.readAIGCInfo(info);
  
  reader.prepare(xmpWebp);
  bool xmpResult = reader.readAIGCInfo(info);
  
  // Then each file should be processed correctly
  ASSERT_THAT(emptyResult, IsFalse()); // empty has no XMP
  ASSERT_THAT(xmpResult, IsTrue());    // xmp file has data
  ASSERT_THAT(info.label, Eq("1"));
}

// ============================================================================
// Behavior: Reader should handle edge cases gracefully
// ============================================================================

TEST_F(WebpAIGCReaderTest, ReaderShouldHandleEmptyFilePath) {
  // Given an empty file path
  std::string emptyPath = "";
  
  // When attempting to prepare
  bool result = reader.prepare(emptyPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(WebpAIGCReaderTest, ReaderShouldMaintainStateAfterFailedRead) {
  // Given a WebP without XMP
  auto emptyWebp = finder.find("webp_empty.webp");
  reader.prepare(emptyWebp);
  
  // When attempting a failed read followed by successful preparation
  bool failedRead = reader.readAIGCInfo(info);
  
  auto xmpWebp = finder.find("webp_with_xmp.webp");
  reader.prepare(xmpWebp);
  bool successRead = reader.readAIGCInfo(info);
  
  // Then the reader should recover and work correctly
  ASSERT_THAT(failedRead, IsFalse());
  ASSERT_THAT(successRead, IsTrue());
}

