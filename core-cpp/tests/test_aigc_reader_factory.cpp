//
// Unit tests for AIGCReaderFactory
// Testing behavior: factory creates correct readers and handles edge cases
//

#include "gimt/gimt_aigc_reader_factory.h"
#include "gimt/gimt_image_format.h"
#include "gimt_testing_resource_finder.h"
#include <gtest/gtest.h>

using namespace gimt;
using namespace gimt_testing;

class AIGCReaderFactoryTest : public ::testing::Test {
protected:
  ResourcePathFinder finder;
  
  std::string getResourcePath(const std::string& filename) {
    return finder.find(filename);
  }
};

// ============================================================================
// 测试行为：根据格式创建正确的 Reader
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldCreateJpegReaderForJpegFormat) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreatePngReaderForPngFormat) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::PNG);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::PNG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateWebpReaderForWebpFormat) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::WEBP);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::WEBP, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateHeifReaderForHeifFormat) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::HEIF);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::HEIF, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldReturnNullptrForUnknownFormat) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::UNKNOWN);
  EXPECT_EQ(nullptr, reader);
}

// ============================================================================
// 测试行为：从文件路径创建 Reader（基于扩展名）
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldCreateJpegReaderFromJpgPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.jpg");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateJpegReaderFromJpegPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.jpeg");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreatePngReaderFromPngPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.png");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::PNG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateWebpReaderFromWebpPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.webp");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::WEBP, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateHeifReaderFromHeifPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.heif");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::HEIF, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateHeifReaderFromHeicPath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.heic");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::HEIF, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldReturnNullptrForUnsupportedExtension) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image.bmp");
  EXPECT_EQ(nullptr, reader);
}

TEST_F(AIGCReaderFactoryTest, ShouldHandleCaseInsensitiveExtensions) {
  auto reader1 = AIGCReaderFactory::createReaderFromPath("image.JPG");
  ASSERT_NE(nullptr, reader1);
  EXPECT_EQ(ImageFormat::JPEG, reader1->getFormat());
  
  auto reader2 = AIGCReaderFactory::createReaderFromPath("image.PNG");
  ASSERT_NE(nullptr, reader2);
  EXPECT_EQ(ImageFormat::PNG, reader2->getFormat());
}

// ============================================================================
// 测试行为：从文件内容创建 Reader（基于文件头）
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldCreateJpegReaderFromJpegFileContent) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto reader = AIGCReaderFactory::createReaderFromContent(jpegFile);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreatePngReaderFromPngFileContent) {
  std::string pngFile = getResourcePath("png_empty.png");
  auto reader = AIGCReaderFactory::createReaderFromContent(pngFile);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::PNG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateWebpReaderFromWebpFileContent) {
  std::string webpFile = getResourcePath("webp_empty.webp");
  auto reader = AIGCReaderFactory::createReaderFromContent(webpFile);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::WEBP, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateHeifReaderFromHeifFileContent) {
  std::string heifFile = getResourcePath("heif_empty.heif");
  auto reader = AIGCReaderFactory::createReaderFromContent(heifFile);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::HEIF, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldReturnNullptrForNonExistentFile) {
  auto reader = AIGCReaderFactory::createReaderFromContent("nonexistent.jpg");
  EXPECT_EQ(nullptr, reader);
}

// ============================================================================
// 测试行为：一步式创建并准备 Reader
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldCreateAndPrepareJpegReaderSuccessfully) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto reader = AIGCReaderFactory::createAndPrepare(jpegFile, true);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
  EXPECT_TRUE(reader->isPrepared());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateAndPreparePngReaderSuccessfully) {
  std::string pngFile = getResourcePath("png_empty.png");
  auto reader = AIGCReaderFactory::createAndPrepare(pngFile, true);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::PNG, reader->getFormat());
  EXPECT_TRUE(reader->isPrepared());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateAndPrepareWebpReaderSuccessfully) {
  std::string webpFile = getResourcePath("webp_empty.webp");
  auto reader = AIGCReaderFactory::createAndPrepare(webpFile, true);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::WEBP, reader->getFormat());
  EXPECT_TRUE(reader->isPrepared());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateAndPrepareHeifReaderSuccessfully) {
  std::string heifFile = getResourcePath("heif_empty.heif");
  auto reader = AIGCReaderFactory::createAndPrepare(heifFile, true);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::HEIF, reader->getFormat());
  EXPECT_TRUE(reader->isPrepared());
}

TEST_F(AIGCReaderFactoryTest, ShouldReturnNullptrWhenPrepareFailsForNonExistentFile) {
  auto reader = AIGCReaderFactory::createAndPrepare("nonexistent.jpg", true);
  EXPECT_EQ(nullptr, reader);
}

TEST_F(AIGCReaderFactoryTest, ShouldUseContentDetectionWhenAutoDetectIsTrue) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto reader = AIGCReaderFactory::createAndPrepare(jpegFile, true);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldUsePathDetectionWhenAutoDetectIsFalse) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto reader = AIGCReaderFactory::createAndPrepare(jpegFile, false);
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

// ============================================================================
// 测试行为：创建的 Reader 可以正常工作
// ============================================================================

TEST_F(AIGCReaderFactoryTest, CreatedReaderShouldBeAbleToReadAIGCInfo) {
  std::string jpegFile = getResourcePath("jpg_with_xmp.jpg");
  auto reader = AIGCReaderFactory::createAndPrepare(jpegFile, true);
  ASSERT_NE(nullptr, reader);
  
  AIGCInfo info;
  bool result = reader->readAIGCInfo(info);
  EXPECT_TRUE(result);
  EXPECT_EQ("1", info.label);  // 根据实际测试文件中的值
}

TEST_F(AIGCReaderFactoryTest, CreatedReaderShouldHandleFilesWithoutAIGCInfo) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto reader = AIGCReaderFactory::createAndPrepare(jpegFile, true);
  ASSERT_NE(nullptr, reader);
  
  AIGCInfo info;
  bool result = reader->readAIGCInfo(info);
  EXPECT_FALSE(result);
}

// ============================================================================
// 测试行为：多次创建 Reader
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldCreateMultipleIndependentReaders) {
  auto reader1 = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  auto reader2 = AIGCReaderFactory::createReader(ImageFormat::PNG);
  auto reader3 = AIGCReaderFactory::createReader(ImageFormat::WEBP);
  
  ASSERT_NE(nullptr, reader1);
  ASSERT_NE(nullptr, reader2);
  ASSERT_NE(nullptr, reader3);
  
  EXPECT_EQ(ImageFormat::JPEG, reader1->getFormat());
  EXPECT_EQ(ImageFormat::PNG, reader2->getFormat());
  EXPECT_EQ(ImageFormat::WEBP, reader3->getFormat());
}

TEST_F(AIGCReaderFactoryTest, ShouldCreateMultipleReadersForSameFormat) {
  auto reader1 = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  auto reader2 = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  
  ASSERT_NE(nullptr, reader1);
  ASSERT_NE(nullptr, reader2);
  EXPECT_EQ(ImageFormat::JPEG, reader1->getFormat());
  EXPECT_EQ(ImageFormat::JPEG, reader2->getFormat());
}

// ============================================================================
// 测试行为：边界情况和错误处理
// ============================================================================

TEST_F(AIGCReaderFactoryTest, ShouldHandleEmptyFilePath) {
  auto reader = AIGCReaderFactory::createReaderFromPath("");
  EXPECT_EQ(nullptr, reader);
}

TEST_F(AIGCReaderFactoryTest, ShouldHandlePathWithoutExtension) {
  auto reader = AIGCReaderFactory::createReaderFromPath("image");
  EXPECT_EQ(nullptr, reader);
}

TEST_F(AIGCReaderFactoryTest, ShouldReturnNullptrWhenCreateAndPrepareFailsForUnsupportedFormat) {
  auto reader = AIGCReaderFactory::createAndPrepare("image.bmp", false);
  EXPECT_EQ(nullptr, reader);
}

TEST_F(AIGCReaderFactoryTest, CreatedReaderShouldNotBePreparedInitially) {
  auto reader = AIGCReaderFactory::createReader(ImageFormat::JPEG);
  ASSERT_NE(nullptr, reader);
  EXPECT_FALSE(reader->isPrepared());
}

TEST_F(AIGCReaderFactoryTest, ShouldHandleComplexFilePaths) {
  auto reader = AIGCReaderFactory::createReaderFromPath("/path/to/some/directory/image.jpg");
  ASSERT_NE(nullptr, reader);
  EXPECT_EQ(ImageFormat::JPEG, reader->getFormat());
}

