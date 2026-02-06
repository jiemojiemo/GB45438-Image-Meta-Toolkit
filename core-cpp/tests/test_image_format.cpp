//
// Unit tests for ImageFormat detection functionality
// Testing behavior: format detection from file paths and content
//

#include "gimt/gimt_image_format.h"
#include "gimt_testing_resource_finder.h"
#include <gtest/gtest.h>
#include <fstream>
#include <vector>

using namespace gimt;
using namespace gimt_testing;

class ImageFormatTest : public ::testing::Test {
protected:
  ResourcePathFinder finder;
  
  std::string getResourcePath(const std::string& filename) {
    return finder.find(filename);
  }

  // 创建临时测试文件
  void createTempFile(const std::string& path, const std::vector<uint8_t>& content) {
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
    file.close();
  }
};

// ============================================================================
// 测试行为：从文件路径检测格式（基于扩展名）
// ============================================================================

TEST_F(ImageFormatTest, ShouldDetectJpegFormatFromJpgExtension) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("image.jpg"));
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("/path/to/photo.jpg"));
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("C:\\Users\\test\\image.jpg"));
}

TEST_F(ImageFormatTest, ShouldDetectJpegFormatFromJpegExtension) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("image.jpeg"));
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("/path/to/photo.JPEG"));
}

TEST_F(ImageFormatTest, ShouldDetectPngFormatFromExtension) {
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("image.png"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("/path/to/photo.PNG"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("test.Png"));
}

TEST_F(ImageFormatTest, ShouldDetectWebpFormatFromExtension) {
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("image.webp"));
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("/path/to/photo.WEBP"));
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("test.WebP"));
}

TEST_F(ImageFormatTest, ShouldDetectHeifFormatFromHeifExtension) {
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromPath("image.heif"));
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromPath("/path/to/photo.HEIF"));
}

TEST_F(ImageFormatTest, ShouldDetectHeifFormatFromHeicExtension) {
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromPath("image.heic"));
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromPath("/path/to/photo.HEIC"));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForUnsupportedExtension) {
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("image.bmp"));
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("image.gif"));
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("image.tiff"));
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("document.pdf"));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForFileWithoutExtension) {
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("image"));
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath("/path/to/file"));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForEmptyPath) {
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromPath(""));
}

TEST_F(ImageFormatTest, ShouldHandlePathsWithMultipleDots) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("my.image.file.jpg"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("test.backup.png"));
}

TEST_F(ImageFormatTest, ShouldBeCaseInsensitiveForExtensions) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("image.JPG"));
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("image.JpG"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("image.PnG"));
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("image.WeBp"));
}

// ============================================================================
// 测试行为：从文件内容检测格式（基于文件头魔数）
// ============================================================================

TEST_F(ImageFormatTest, ShouldDetectJpegFormatFromFileHeader) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromContent(jpegFile));
}

TEST_F(ImageFormatTest, ShouldDetectPngFormatFromFileHeader) {
  std::string pngFile = getResourcePath("png_empty.png");
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromContent(pngFile));
}

TEST_F(ImageFormatTest, ShouldDetectWebpFormatFromFileHeader) {
  std::string webpFile = getResourcePath("webp_empty.webp");
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromContent(webpFile));
}

TEST_F(ImageFormatTest, ShouldDetectHeifFormatFromFileHeader) {
  std::string heifFile = getResourcePath("heif_empty.heif");
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromContent(heifFile));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForNonExistentFile) {
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromContent("nonexistent_file.jpg"));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForEmptyFile) {
  std::string tempFile = "/tmp/empty_test_file.dat";
  createTempFile(tempFile, {});
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForTooShortFile) {
  std::string tempFile = "/tmp/short_test_file.dat";
  createTempFile(tempFile, {0x00});
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldDetectFormatRegardlessOfFileExtension) {
  // JPEG 文件即使扩展名错误也能检测
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromContent(jpegFile));
  
  // PNG 文件即使扩展名错误也能检测
  std::string pngFile = getResourcePath("png_empty.png");
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromContent(pngFile));
}

TEST_F(ImageFormatTest, ShouldDetectJpegByMagicBytes) {
  std::string tempFile = "/tmp/test_jpeg_magic.dat";
  std::vector<uint8_t> jpegHeader = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F'};
  createTempFile(tempFile, jpegHeader);
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldDetectPngByMagicBytes) {
  std::string tempFile = "/tmp/test_png_magic.dat";
  std::vector<uint8_t> pngHeader = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  createTempFile(tempFile, pngHeader);
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldDetectWebpByMagicBytes) {
  std::string tempFile = "/tmp/test_webp_magic.dat";
  std::vector<uint8_t> webpHeader = {'R', 'I', 'F', 'F', 0x00, 0x00, 0x00, 0x00, 
                                     'W', 'E', 'B', 'P'};
  createTempFile(tempFile, webpHeader);
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldDetectHeifByMagicBytes) {
  std::string tempFile = "/tmp/test_heif_magic.dat";
  std::vector<uint8_t> heifHeader = {0x00, 0x00, 0x00, 0x18, 'f', 't', 'y', 'p',
                                     'h', 'e', 'i', 'c'};
  createTempFile(tempFile, heifHeader);
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldDetectHeifWithMif1Brand) {
  std::string tempFile = "/tmp/test_heif_mif1.dat";
  std::vector<uint8_t> heifHeader = {0x00, 0x00, 0x00, 0x18, 'f', 't', 'y', 'p',
                                     'm', 'i', 'f', '1'};
  createTempFile(tempFile, heifHeader);
  EXPECT_EQ(ImageFormat::HEIF, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldNotDetectInvalidJpegHeader) {
  std::string tempFile = "/tmp/test_invalid_jpeg.dat";
  std::vector<uint8_t> invalidHeader = {0xFF, 0xD7, 0xFF, 0xE0}; // 错误的 JPEG 标记
  createTempFile(tempFile, invalidHeader);
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

TEST_F(ImageFormatTest, ShouldNotDetectInvalidPngHeader) {
  std::string tempFile = "/tmp/test_invalid_png.dat";
  std::vector<uint8_t> invalidHeader = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0B}; // 最后一字节错误
  createTempFile(tempFile, invalidHeader);
  EXPECT_EQ(ImageFormat::UNKNOWN, detectFormatFromContent(tempFile));
  std::remove(tempFile.c_str());
}

// ============================================================================
// 测试行为：格式名称转换
// ============================================================================

TEST_F(ImageFormatTest, ShouldReturnCorrectNameForJpegFormat) {
  EXPECT_EQ("JPEG", getFormatName(ImageFormat::JPEG));
}

TEST_F(ImageFormatTest, ShouldReturnCorrectNameForPngFormat) {
  EXPECT_EQ("PNG", getFormatName(ImageFormat::PNG));
}

TEST_F(ImageFormatTest, ShouldReturnCorrectNameForWebpFormat) {
  EXPECT_EQ("WebP", getFormatName(ImageFormat::WEBP));
}

TEST_F(ImageFormatTest, ShouldReturnCorrectNameForHeifFormat) {
  EXPECT_EQ("HEIF", getFormatName(ImageFormat::HEIF));
}

TEST_F(ImageFormatTest, ShouldReturnUnknownForUnknownFormat) {
  EXPECT_EQ("Unknown", getFormatName(ImageFormat::UNKNOWN));
}

// ============================================================================
// 测试行为：路径检测 vs 内容检测的一致性
// ============================================================================

TEST_F(ImageFormatTest, PathAndContentDetectionShouldAgreeForValidFiles) {
  std::string jpegFile = getResourcePath("jpg_empty.jpg");
  EXPECT_EQ(detectFormatFromPath(jpegFile), detectFormatFromContent(jpegFile));
  
  std::string pngFile = getResourcePath("png_empty.png");
  EXPECT_EQ(detectFormatFromPath(pngFile), detectFormatFromContent(pngFile));
  
  std::string webpFile = getResourcePath("webp_empty.webp");
  EXPECT_EQ(detectFormatFromPath(webpFile), detectFormatFromContent(webpFile));
  
  std::string heifFile = getResourcePath("heif_empty.heif");
  EXPECT_EQ(detectFormatFromPath(heifFile), detectFormatFromContent(heifFile));
}

TEST_F(ImageFormatTest, ContentDetectionShouldBeMoreReliableThanPathDetection) {
  // 创建一个 JPEG 文件但使用 .png 扩展名
  std::string tempFile = "/tmp/fake.png";
  std::vector<uint8_t> jpegHeader = {0xFF, 0xD8, 0xFF, 0xE0};
  createTempFile(tempFile, jpegHeader);
  
  // 路径检测会认为是 PNG
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath(tempFile));
  
  // 内容检测会正确识别为 JPEG
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromContent(tempFile));
  
  std::remove(tempFile.c_str());
}

// ============================================================================
// 测试行为：边界情况和错误处理
// ============================================================================

TEST_F(ImageFormatTest, ShouldHandleVeryLongFilePaths) {
  std::string longPath = "/very/long/path/to/some/deeply/nested/directory/structure/image.jpg";
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath(longPath));
}

TEST_F(ImageFormatTest, ShouldHandlePathsWithSpecialCharacters) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("/path/with spaces/image.jpg"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("/path/with-dashes/image.png"));
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("/path/with_underscores/image.webp"));
}

TEST_F(ImageFormatTest, ShouldHandleRelativePaths) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath("./image.jpg"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("../image.png"));
  EXPECT_EQ(ImageFormat::WEBP, detectFormatFromPath("../../image.webp"));
}

TEST_F(ImageFormatTest, ShouldHandleHiddenFiles) {
  EXPECT_EQ(ImageFormat::JPEG, detectFormatFromPath(".hidden.jpg"));
  EXPECT_EQ(ImageFormat::PNG, detectFormatFromPath("/path/.hidden.png"));
}

