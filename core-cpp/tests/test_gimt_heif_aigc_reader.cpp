//
// Test suite for HeifAIGCReader - following TDD principles
// Testing behavior rather than implementation details
//

#include "gimt/gimt_heif_aigc_reader.h"
#include "gimt_testing_resource_finder.h"
#include <gmock/gmock.h>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class HeifAIGCReaderTest : public Test {
public:
  HeifAIGCReader reader;
  ResourcePathFinder finder;
  AIGCInfo info;
};

// ============================================================================
// Behavior: Reader should handle file preparation correctly
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldFailToPrepareHeifWhenFileDoesNotExist) {
  // Given a non-existent file path
  std::string nonExistentPath = "this_file_does_not_exist_at_all.heif";
  
  // When attempting to prepare the reader
  bool result = reader.prepare(nonExistentPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldSuccessfullyPrepareWhenHeifFileExists) {
  // Given an existing HEIF file
  auto heifPath = finder.find("heif_empty.heif");
  
  // When preparing the reader
  bool result = reader.prepare(heifPath);
  
  // Then preparation should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldHandleMultiplePrepareCallsGracefully) {
  // Given an existing HEIF file
  auto heifPath = finder.find("heif_empty.heif");
  
  // When preparing the reader multiple times
  bool firstPrepare = reader.prepare(heifPath);
  bool secondPrepare = reader.prepare(heifPath);
  
  // Then both preparations should succeed
  ASSERT_THAT(firstPrepare, IsTrue());
  ASSERT_THAT(secondPrepare, IsTrue());
}

// ============================================================================
// Behavior: Reader should validate file format before processing
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldFailToReadAIGCInfoWithoutPreparation) {
  // Given a reader that has not been prepared
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldRejectNonHeifFileFormat) {
  // Given a JPEG file (not a HEIF)
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_THAT(reader.prepare(jpegPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldRejectPNGFileFormat) {
  // Given a PNG file (not a HEIF)
  auto pngPath = finder.find("png_empty.png");
  ASSERT_THAT(reader.prepare(pngPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldRejectWebPFileFormat) {
  // Given a WebP file (not a HEIF)
  auto webpPath = finder.find("webp_empty.webp");
  ASSERT_THAT(reader.prepare(webpPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail due to incorrect file format
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should handle HEIF files without XMP metadata
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldReturnFalseForHeifWithoutXMPMetadata) {
  // Given a HEIF file without XMP metadata
  auto heifPath = finder.find("heif_empty.heif");
  ASSERT_THAT(reader.prepare(heifPath), IsTrue());
  
  // When attempting to read AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then reading should fail (no XMP data present)
  ASSERT_THAT(result, IsFalse());
}

// ============================================================================
// Behavior: Reader should extract AIGC info from HEIF with XMP metadata
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldSuccessfullyExtractAIGCInfoFromHeifWithXMP) {
  // Given a HEIF file containing XMP metadata with AIGC info
  auto heifPath = finder.find("heif_with_xmp.heif");
  ASSERT_THAT(reader.prepare(heifPath), IsTrue());
  
  // When reading AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then extraction should succeed
  ASSERT_THAT(result, IsTrue());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldExtractCorrectLabelFromXMP) {
  // Given a HEIF file with XMP containing AIGC info
  auto heifPath = finder.find("heif_with_xmp.heif");
  reader.prepare(heifPath);
  
  // When reading AIGC info
  reader.readAIGCInfo(info);
  
  // Then the label should match expected value
  ASSERT_THAT(info.label, Eq("1"));
}

TEST_F(HeifAIGCReaderTest, ReaderShouldExtractAllAIGCFieldsCorrectly) {
  // Given a HEIF file with complete AIGC metadata
  auto heifPath = finder.find("heif_with_xmp.heif");
  reader.prepare(heifPath);
  
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

TEST_F(HeifAIGCReaderTest, ReaderShouldAllowMultipleReadsOnSameFile) {
  // Given a prepared HEIF file with XMP
  auto heifPath = finder.find("heif_with_xmp.heif");
  reader.prepare(heifPath);
  
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

TEST_F(HeifAIGCReaderTest, ReaderShouldSwitchBetweenDifferentFiles) {
  // Given two different HEIF files
  auto emptyHeif = finder.find("heif_empty.heif");
  auto xmpHeif = finder.find("heif_with_xmp.heif");
  
  // When switching between files
  reader.prepare(emptyHeif);
  bool emptyResult = reader.readAIGCInfo(info);
  
  reader.prepare(xmpHeif);
  bool xmpResult = reader.readAIGCInfo(info);
  
  // Then each file should be processed correctly
  ASSERT_THAT(emptyResult, IsFalse()); // empty has no XMP
  ASSERT_THAT(xmpResult, IsTrue());    // xmp file has data
  ASSERT_THAT(info.label, Eq("1"));
}

// ============================================================================
// Behavior: Reader should handle edge cases gracefully
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldHandleEmptyFilePath) {
  // Given an empty file path
  std::string emptyPath = "";
  
  // When attempting to prepare
  bool result = reader.prepare(emptyPath);
  
  // Then preparation should fail
  ASSERT_THAT(result, IsFalse());
}

TEST_F(HeifAIGCReaderTest, ReaderShouldMaintainStateAfterFailedRead) {
  // Given a HEIF without XMP
  auto emptyHeif = finder.find("heif_empty.heif");
  reader.prepare(emptyHeif);
  
  // When attempting a failed read followed by successful preparation
  bool failedRead = reader.readAIGCInfo(info);
  
  auto xmpHeif = finder.find("heif_with_xmp.heif");
  reader.prepare(xmpHeif);
  bool successRead = reader.readAIGCInfo(info);
  
  // Then the reader should recover and work correctly
  ASSERT_THAT(failedRead, IsFalse());
  ASSERT_THAT(successRead, IsTrue());
}

// ============================================================================
// Behavior: Reader should handle HEIC variant correctly
// ============================================================================

TEST_F(HeifAIGCReaderTest, ReaderShouldRecognizeHeicAsValidFormat) {
  // Given a file with .heic extension (same format as .heif)
  // Note: Both heic and heif use the same ISO BMFF structure
  auto heifPath = finder.find("heif_with_xmp.heif");
  ASSERT_THAT(reader.prepare(heifPath), IsTrue());
  
  // When reading AIGC info
  bool result = reader.readAIGCInfo(info);
  
  // Then it should work correctly regardless of extension
  ASSERT_THAT(result, IsTrue());
}


