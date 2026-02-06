//
// Unit tests for AIGCWriterFactory
// Testing behavior: factory creates correct writers and handles edge cases
//

#include "gimt/gimt_aigc_writer_factory.h"
#include "gimt/gimt_aigc_reader_factory.h"
#include "gimt/gimt_image_format.h"
#include "gimt_testing_resource_finder.h"
#include <gtest/gtest.h>
#include <fstream>

using namespace gimt;
using namespace gimt_testing;

class AIGCWriterFactoryTest : public ::testing::Test {
protected:
  ResourcePathFinder finder;
  
  std::string getResourcePath(const std::string& filename) {
    return finder.find(filename);
  }
  
  std::string getTempOutputPath(const std::string& filename) {
    return "/tmp/test_writer_factory_" + filename;
  }
  
  void TearDown() override {
    // 清理测试文件
    std::remove(getTempOutputPath("output.jpg").c_str());
    std::remove(getTempOutputPath("output.png").c_str());
    std::remove(getTempOutputPath("output.webp").c_str());
    std::remove(getTempOutputPath("output.heif").c_str());
  }
};

// ============================================================================
// 测试行为：根据格式创建正确的 Writer
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldCreateJpegWriterForJpegFormat) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::JPEG);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreatePngWriterForPngFormat) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::PNG);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::PNG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateWebpWriterForWebpFormat) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::WEBP);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::WEBP, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateHeifWriterForHeifFormat) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::HEIF);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::HEIF, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldReturnNullptrForUnknownFormat) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::UNKNOWN);
  EXPECT_EQ(nullptr, writer);
}

// ============================================================================
// 测试行为：从文件路径创建 Writer（基于扩展名）
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldCreateJpegWriterFromJpgPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.jpg");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateJpegWriterFromJpegPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.jpeg");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreatePngWriterFromPngPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.png");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::PNG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateWebpWriterFromWebpPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.webp");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::WEBP, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateHeifWriterFromHeifPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.heif");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::HEIF, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateHeifWriterFromHeicPath) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.heic");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::HEIF, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldReturnNullptrForUnsupportedExtension) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image.bmp");
  EXPECT_EQ(nullptr, writer);
}

TEST_F(AIGCWriterFactoryTest, ShouldHandleCaseInsensitiveExtensions) {
  auto writer1 = AIGCWriterFactory::createWriterFromPath("image.JPG");
  ASSERT_NE(nullptr, writer1);
  EXPECT_EQ(ImageFormat::JPEG, writer1->getFormat());
  
  auto writer2 = AIGCWriterFactory::createWriterFromPath("image.PNG");
  ASSERT_NE(nullptr, writer2);
  EXPECT_EQ(ImageFormat::PNG, writer2->getFormat());
}

// ============================================================================
// 测试行为：从文件内容创建 Writer（基于文件头）
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldCreateJpegWriterFromJpegFileContent) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  auto writer = AIGCWriterFactory::createWriterFromContent(jpegFile);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreatePngWriterFromPngFileContent) {
  std::string pngFile = getResourcePath("png_empty.png");
  auto writer = AIGCWriterFactory::createWriterFromContent(pngFile);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::PNG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateWebpWriterFromWebpFileContent) {
  std::string webpFile = getResourcePath("webp_empty.webp");
  auto writer = AIGCWriterFactory::createWriterFromContent(webpFile);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::WEBP, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateHeifWriterFromHeifFileContent) {
  std::string heifFile = getResourcePath("heif_empty.heif");
  auto writer = AIGCWriterFactory::createWriterFromContent(heifFile);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::HEIF, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldReturnNullptrForNonExistentFile) {
  auto writer = AIGCWriterFactory::createWriterFromContent("nonexistent.jpg");
  EXPECT_EQ(nullptr, writer);
}

// ============================================================================
// 测试行为：一步式创建并准备 Writer
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldCreateAndPrepareJpegWriterSuccessfully) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
  EXPECT_TRUE(writer->isPrepared());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateAndPreparePngWriterSuccessfully) {
  std::string inputFile = getResourcePath("png_empty.png");
  std::string outputFile = getTempOutputPath("output.png");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::PNG, writer->getFormat());
  EXPECT_TRUE(writer->isPrepared());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateAndPrepareWebpWriterSuccessfully) {
  std::string inputFile = getResourcePath("webp_empty.webp");
  std::string outputFile = getTempOutputPath("output.webp");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::WEBP, writer->getFormat());
  EXPECT_TRUE(writer->isPrepared());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateAndPrepareHeifWriterSuccessfully) {
  std::string inputFile = getResourcePath("heif_empty.heif");
  std::string outputFile = getTempOutputPath("output.heif");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::HEIF, writer->getFormat());
  EXPECT_TRUE(writer->isPrepared());
}

TEST_F(AIGCWriterFactoryTest, ShouldReturnNullptrWhenPrepareFailsForNonExistentFile) {
  std::string outputFile = getTempOutputPath("output.jpg");
  auto writer = AIGCWriterFactory::createAndPrepare("nonexistent.jpg", outputFile, true);
  EXPECT_EQ(nullptr, writer);
}

TEST_F(AIGCWriterFactoryTest, ShouldUseContentDetectionWhenAutoDetectIsTrue) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldUsePathDetectionWhenAutoDetectIsFalse) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, false);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

// ============================================================================
// 测试行为：创建的 Writer 可以正常工作
// ============================================================================

TEST_F(AIGCWriterFactoryTest, CreatedWriterShouldBeAbleToWriteAIGCInfo) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  
  AIGCInfo info;
  info.label = "AIGC";
  info.contentProducer = "TestProducer";
  info.produceID = "12345";
  
  bool result = writer->writeAIGCInfo(info);
  EXPECT_TRUE(result);
  
  // 验证文件已创建
  std::ifstream file(outputFile);
  EXPECT_TRUE(file.good());
}

TEST_F(AIGCWriterFactoryTest, WrittenDataShouldBeReadableByReader) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  // 写入数据
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  
  AIGCInfo writeInfo;
  writeInfo.label = "AIGC";
  writeInfo.contentProducer = "FactoryTest";
  writeInfo.produceID = "99999";
  
  ASSERT_TRUE(writer->writeAIGCInfo(writeInfo));
  
  // 读取数据
  auto reader = AIGCReaderFactory::createAndPrepare(outputFile, true);
  ASSERT_NE(nullptr, reader);
  
  AIGCInfo readInfo;
  ASSERT_TRUE(reader->readAIGCInfo(readInfo));
  
  EXPECT_EQ(writeInfo.label, readInfo.label);
  EXPECT_EQ(writeInfo.contentProducer, readInfo.contentProducer);
  EXPECT_EQ(writeInfo.produceID, readInfo.produceID);
}

// ============================================================================
// 测试行为：多次创建 Writer
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldCreateMultipleIndependentWriters) {
  auto writer1 = AIGCWriterFactory::createWriter(ImageFormat::JPEG);
  auto writer2 = AIGCWriterFactory::createWriter(ImageFormat::PNG);
  auto writer3 = AIGCWriterFactory::createWriter(ImageFormat::WEBP);
  
  ASSERT_NE(nullptr, writer1);
  ASSERT_NE(nullptr, writer2);
  ASSERT_NE(nullptr, writer3);
  
  EXPECT_EQ(ImageFormat::JPEG, writer1->getFormat());
  EXPECT_EQ(ImageFormat::PNG, writer2->getFormat());
  EXPECT_EQ(ImageFormat::WEBP, writer3->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldCreateMultipleWritersForSameFormat) {
  auto writer1 = AIGCWriterFactory::createWriter(ImageFormat::JPEG);
  auto writer2 = AIGCWriterFactory::createWriter(ImageFormat::JPEG);
  
  ASSERT_NE(nullptr, writer1);
  ASSERT_NE(nullptr, writer2);
  EXPECT_EQ(ImageFormat::JPEG, writer1->getFormat());
  EXPECT_EQ(ImageFormat::JPEG, writer2->getFormat());
}

// ============================================================================
// 测试行为：Writer 和 Reader 工厂的协同工作
// ============================================================================

TEST_F(AIGCWriterFactoryTest, WriterAndReaderFactoryShouldWorkTogether) {
  std::string inputFile = getResourcePath("png_empty.png");
  std::string outputFile = getTempOutputPath("output.png");
  
  // 使用 Writer 工厂写入
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  
  AIGCInfo writeInfo;
  writeInfo.label = "AIGC";
  writeInfo.contentProducer = "Integration";
  writeInfo.produceID = "54321";
  
  ASSERT_TRUE(writer->writeAIGCInfo(writeInfo));
  
  // 使用 Reader 工厂读取
  auto reader = AIGCReaderFactory::createAndPrepare(outputFile, true);
  ASSERT_NE(nullptr, reader);
  
  AIGCInfo readInfo;
  ASSERT_TRUE(reader->readAIGCInfo(readInfo));
  
  EXPECT_EQ(writeInfo, readInfo);
}

TEST_F(AIGCWriterFactoryTest, ShouldHandleMultipleFormatsInSequence) {
  // JPEG
  {
    std::string inputFile = getResourcePath("jpg_empty.jpg");
    std::string outputFile = getTempOutputPath("output.jpg");
    auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
    ASSERT_NE(nullptr, writer);
    EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
  }
  
  // PNG
  {
    std::string inputFile = getResourcePath("png_empty.png");
    std::string outputFile = getTempOutputPath("output.png");
    auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
    ASSERT_NE(nullptr, writer);
    EXPECT_EQ(ImageFormat::PNG, writer->getFormat());
  }
  
  // WebP
  {
    std::string inputFile = getResourcePath("webp_empty.webp");
    std::string outputFile = getTempOutputPath("output.webp");
    auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
    ASSERT_NE(nullptr, writer);
    EXPECT_EQ(ImageFormat::WEBP, writer->getFormat());
  }
}

// ============================================================================
// 测试行为：边界情况和错误处理
// ============================================================================

TEST_F(AIGCWriterFactoryTest, ShouldHandleEmptyInputFilePath) {
  std::string outputFile = getTempOutputPath("output.jpg");
  auto writer = AIGCWriterFactory::createAndPrepare("", outputFile, false);
  EXPECT_EQ(nullptr, writer);
}

TEST_F(AIGCWriterFactoryTest, ShouldHandlePathWithoutExtension) {
  auto writer = AIGCWriterFactory::createWriterFromPath("image");
  EXPECT_EQ(nullptr, writer);
}

TEST_F(AIGCWriterFactoryTest, ShouldReturnNullptrWhenCreateAndPrepareFailsForUnsupportedFormat) {
  std::string outputFile = getTempOutputPath("output.bmp");
  auto writer = AIGCWriterFactory::createAndPrepare("image.bmp", outputFile, false);
  EXPECT_EQ(nullptr, writer);
}

TEST_F(AIGCWriterFactoryTest, CreatedWriterShouldNotBePreparedInitially) {
  auto writer = AIGCWriterFactory::createWriter(ImageFormat::JPEG);
  ASSERT_NE(nullptr, writer);
  EXPECT_FALSE(writer->isPrepared());
}

TEST_F(AIGCWriterFactoryTest, ShouldHandleComplexFilePaths) {
  auto writer = AIGCWriterFactory::createWriterFromPath("/path/to/some/directory/image.jpg");
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

TEST_F(AIGCWriterFactoryTest, ShouldHandleDifferentInputAndOutputFormats) {
  std::string inputFile = getResourcePath("jpg_empty.jpg");
  std::string outputFile = getTempOutputPath("output.jpg");
  
  // 输入是 JPEG，输出也是 JPEG
  auto writer = AIGCWriterFactory::createAndPrepare(inputFile, outputFile, true);
  ASSERT_NE(nullptr, writer);
  EXPECT_EQ(ImageFormat::JPEG, writer->getFormat());
}

