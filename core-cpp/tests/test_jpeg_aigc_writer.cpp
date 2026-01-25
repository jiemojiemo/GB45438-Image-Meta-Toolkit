//
// Created by user on 1/25/26.
//

#include "gimt/gimt_binary_reader.h"
#include "gimt/gimt_patter_matcher.h"
#include "gimt/gimt_jpeg_aigc_reader.h"
#include "gimt/gimt_jpeg_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <fstream>
#include <gmock/gmock.h>
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

  gimt::BinaryReader fileReader(outFile);
  uint8_t marker[2];

  ASSERT_EQ(fileReader.readBytes(marker, 2), 2u);
  ASSERT_EQ(marker[0], 0xFF);
  ASSERT_EQ(marker[1], 0xD8);

  ASSERT_EQ(fileReader.readBytes(marker, 2), 2u);
  ASSERT_EQ(marker[0], 0xFF);
  ASSERT_EQ(marker[1], 0xE0);

  uint16_t app0Len = fileReader.readU16BE();
  ASSERT_GT(app0Len, 2u);
  fileReader.skip(app0Len - 2);

  ASSERT_EQ(fileReader.readBytes(marker, 2), 2u);
  ASSERT_EQ(marker[0], 0xFF);
  ASSERT_EQ(marker[1], 0xE1);

  uint16_t app1Len = fileReader.readU16BE();
  ASSERT_GT(app1Len, 2u);

  const std::string expectedSig = "http://ns.adobe.com/xap/1.0/";
  std::vector<uint8_t> sigBuf(expectedSig.size());
  ASSERT_EQ(fileReader.readBytes(sigBuf.data(), sigBuf.size()), sigBuf.size());
  ASSERT_TRUE(gimt::PatternMatcher::matchString(sigBuf.data(), sigBuf.size(), expectedSig));
}

