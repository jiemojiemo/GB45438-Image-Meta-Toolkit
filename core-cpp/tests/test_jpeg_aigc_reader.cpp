//
// Created by user on 1/25/26.
//

#include "gimt/gimt_jpeg_aigc_reader.h"
#include "gimt_testing_resource_finder.h"
#include <gmock/gmock.h>

using namespace testing;
using namespace gimt;
using namespace gimt_testing;

class JpegAIGCReaderTest : public Test {
public:
  JpegAIGCReader reader;
  ResourcePathFinder finder;
  AIGCInfo info;
};

TEST_F(JpegAIGCReaderTest, PrepareFailedIfFileNotExist) {
  bool result = reader.prepare("non_existent_file.jpg");
  ASSERT_THAT(result, IsFalse());
}

TEST_F(JpegAIGCReaderTest, PrepareOkIfFileExists) {
  auto jpegPath = finder.find("jpg_with_xmp.jpg");
  bool result = reader.prepare(jpegPath);
  ASSERT_THAT(result, IsTrue());
}

TEST_F(JpegAIGCReaderTest, ReadAIGCInfoFailsWithoutPrepare) {
  bool result = reader.readAIGCInfo(info);
  ASSERT_THAT(result, IsFalse());
}

TEST_F(JpegAIGCReaderTest, ReadAIGCInfoFailedIfNotAJpegFile) {
  auto pngPath = finder.find("png_empty.jpg");

  bool result = reader.readAIGCInfo(info);

  ASSERT_FALSE(result);
}

TEST_F(JpegAIGCReaderTest, ReadAIGCInfoExpected) {
  auto jpegPath = finder.find("jpg_with_xmp.jpg");

  reader.prepare(jpegPath);
  auto ok = reader.readAIGCInfo(info);
  ASSERT_TRUE(ok);

  AIGCInfo expected{.label = "1",
                    .contentProducer = "ContentProducer",
                    .produceID = "ProduceID",
                    .reservedCode1 = "ReservedCode1",
                    .contentPropagator = "ContentPropagator",
                    .propagateID = "PropagateID",
                    .reservedCode2 = "ReservedCode2"};

  ASSERT_THAT(info, Eq(expected));
}