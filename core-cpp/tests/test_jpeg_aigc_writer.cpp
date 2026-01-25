//
// Created by user on 1/25/26.
//

#include "gimt/gimt_jpeg_aigc_reader.h"
#include "gimt/gimt_jpeg_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <fstream>
#include <gmock/gmock.h>
#include <iterator>
#include <vector>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class JpegAIGCWriterTest : public Test {
public:
  ResourcePathFinder finder;
  JpegAIGCWriter writer;
  JpegAIGCReader reader;
};

TEST_F(JpegAIGCWriterTest, PrepareFailedIfInputFileNotExist) {
  bool ok = writer.prepare("non_existent_input.jpg", "out.jpg");
  ASSERT_THAT(ok, IsFalse());
}

TEST_F(JpegAIGCWriterTest, PrepareFailedIfInputIsNotJpeg) {
  auto pngPath = finder.find("png_empty.png");
  ASSERT_FALSE(pngPath.empty());

  bool ok = writer.prepare(pngPath, "out_non_jpeg.jpg");
  ASSERT_THAT(ok, IsFalse());
}

TEST_F(JpegAIGCWriterTest, PrepareOkIfValidJpeg) {
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_FALSE(jpegPath.empty());

  bool ok = writer.prepare(jpegPath, "out_valid_jpeg.jpg");
  ASSERT_THAT(ok, IsTrue());
}

TEST_F(JpegAIGCWriterTest, WriteAIGCInfoFailsWithoutPrepare) {
  AIGCInfo info;
  bool ok = writer.writeAIGCInfo(info);
  ASSERT_THAT(ok, IsFalse());
}

TEST_F(JpegAIGCWriterTest, WriteAndReadBackAIGCInfoExpected) {
  auto jpegPath = finder.find("jpg_empty.jpg");
  ASSERT_FALSE(jpegPath.empty());

  const std::string outPath = "writer_output_with_aigc.jpg";

  ASSERT_TRUE(writer.prepare(jpegPath, outPath));

  AIGCInfo expected{.label = "1",
                    .contentProducer = "ContentProducer",
                    .produceID = "ProduceID",
                    .reservedCode1 = "ReservedCode1",
                    .contentPropagator = "ContentPropagator",
                    .propagateID = "PropagateID",
                    .reservedCode2 = "ReservedCode2"};

  ASSERT_TRUE(writer.writeAIGCInfo(expected));

  // 使用现有的 reader 从输出 JPEG 中读取 AIGC 信息，验证行为
  ASSERT_TRUE(reader.prepare(outPath));

  AIGCInfo actual;
  bool readOk = reader.readAIGCInfo(actual);
  ASSERT_TRUE(readOk);
  ASSERT_THAT(actual, Eq(expected));
}

TEST_F(JpegAIGCWriterTest, InsertsApp1AfterApp0WhenApp0Present) {
  auto jpegPath = finder.find("jpg_with_xmp.jpg");
  ASSERT_FALSE(jpegPath.empty());

  const std::string outPath = "writer_output_after_app0.jpg";
  ASSERT_TRUE(writer.prepare(jpegPath, outPath));

  AIGCInfo info{.label = "1",
                .contentProducer = "ContentProducer",
                .produceID = "ProduceID",
                .reservedCode1 = "ReservedCode1",
                .contentPropagator = "ContentPropagator",
                .propagateID = "PropagateID",
                .reservedCode2 = "ReservedCode2"};

  ASSERT_TRUE(writer.writeAIGCInfo(info));

  std::ifstream outFile(outPath, std::ios::binary);
  ASSERT_TRUE(outFile.is_open());

  std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(outFile)),
                                std::istreambuf_iterator<char>());
  ASSERT_GE(fileData.size(), 10u);
  ASSERT_EQ(fileData[0], 0xFF);
  ASSERT_EQ(fileData[1], 0xD8);

  size_t offset = 2;
  ASSERT_LT(offset + 3, fileData.size());
  ASSERT_EQ(fileData[offset], 0xFF);
  ASSERT_EQ(fileData[offset + 1], 0xE0);

  uint16_t app0Len =
      (static_cast<uint16_t>(fileData[offset + 2]) << 8) | fileData[offset + 3];
  size_t app0End = offset + 2 + app0Len;
  ASSERT_LE(app0End, fileData.size());

  size_t nextMarker = app0End;
  ASSERT_LT(nextMarker + 3, fileData.size());
  ASSERT_EQ(fileData[nextMarker], 0xFF);
  ASSERT_EQ(fileData[nextMarker + 1], 0xE1);

  const std::string expectedSig = "http://ns.adobe.com/xap/1.0/";
  size_t sigStart = nextMarker + 4;
  ASSERT_LE(sigStart + expectedSig.size(), fileData.size());
  std::string actualSig(reinterpret_cast<const char *>(&fileData[sigStart]),
                        expectedSig.size());
  ASSERT_EQ(actualSig, expectedSig);
}

