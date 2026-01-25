//
// Created by user on 1/25/26.
//

#include "gimt/gimt_jpeg_aigc_reader.h"
#include "gimt/gimt_jpeg_aigc_writer.h"
#include "gimt_testing_resource_finder.h"
#include <gmock/gmock.h>

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

