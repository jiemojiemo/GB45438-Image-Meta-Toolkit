//
// Created by user on 1/25/26.
//
#include <sstream>
#include "gimt/gimt_binary_reader.h"
#include <gmock/gmock.h>
#include <memory>

using namespace testing;

// 定义 Test Fixture
class BinaryReaderTest : public Test {
protected:
    std::stringstream ss;
    std::unique_ptr<BinaryReader> reader;

    // 辅助函数：快速初始化 reader
    void setupReader(const std::string& data) {
        ss.str(data);
        ss.clear(); // 重置 stream 状态（如 eofbit, failbit）
        reader = std::make_unique<BinaryReader>(ss);
    }
};

// 行为：读取成功时，应返回请求的字节数
TEST_F(BinaryReaderTest, ShouldReturnFullCountOnSuccessfulRead) {
    setupReader({0x01, 0x02, 0x03});
    uint8_t buf[2] = {0};

    size_t readCount = reader->readBytes(buf, 2);

    ASSERT_THAT(readCount, Eq(2));
    ASSERT_THAT(buf[0], Eq(0x01));
    ASSERT_THAT(buf[1], Eq(0x02));
}

// 行为：当流中数据不足时，应返回实际读取到的较小数量
TEST_F(BinaryReaderTest, ShouldReturnActualCountOnPartialRead) {
    setupReader({0x01, 0x02});
    uint8_t buf[5] = {0};

    size_t readCount = reader->readBytes(buf, 5);

    ASSERT_THAT(readCount, Eq(2));
    // 检查缓冲区前两个字节
    ASSERT_THAT(std::vector<uint8_t>(buf, buf + 2), ElementsAre(0x01, 0x02));
}

// 行为：在 EOF 之后尝试读取，应返回 0
TEST_F(BinaryReaderTest, ShouldReturnZeroWhenReadingAtEOF) {
    setupReader({0x01});
    uint8_t buf[1];

    reader->readBytes(buf, 1); // 耗尽数据
    size_t readCount = reader->readBytes(buf, 1);

    ASSERT_THAT(readCount, Eq(0));
    ASSERT_THAT(reader->isEOF(), IsTrue());
}

// 行为：readU16BE 应能正确处理大端序转换
TEST_F(BinaryReaderTest, ShouldReadU16BEInCorrectOrder) {
    setupReader({0x12, 0x34});

    ASSERT_THAT(reader->readU16BE(), Eq(0x1234));
}

// 行为：readU16BE 在数据不足时应返回 0
TEST_F(BinaryReaderTest, U16ReadShouldHandleInsufficientData) {
    setupReader({0x01});

    ASSERT_THAT(reader->readU16BE(), Eq(0));
}

// 行为：readU32BE 应能正确处理大端序转换
TEST_F(BinaryReaderTest, ShouldReadU32BEInCorrectOrder) {
    setupReader({0x12, 0x34, 0x56, 0x78});

    ASSERT_THAT(reader->readU32BE(), Eq(0x12345678));
}

// 行为：readU32BE 在数据不足时应返回 0
TEST_F(BinaryReaderTest, U32ReadShouldHandleInsufficientData) {
    setupReader({0x01, 0x02, 0x03});

    ASSERT_THAT(reader->readU32BE(), Eq(0));
}

// 行为：skip 和 tell 应能正确追踪流指针位置
TEST_F(BinaryReaderTest, ShouldTrackPositionWithTellAndSkip) {
    setupReader("ABCDE"); // 5 bytes

    ASSERT_THAT(reader->tell(), Eq(0));

    reader->skip(2);
    ASSERT_THAT(reader->tell(), Eq(2));

    uint8_t buf[1];
    reader->readBytes(buf, 1);
    ASSERT_THAT(buf[0], Eq('C'));
    ASSERT_THAT(reader->tell(), Eq(3));
}

// 行为：isEOF 应在到达末尾时立即感知（即使还没尝试读取失败）
TEST_F(BinaryReaderTest, ShouldRecognizeEOFImmediately) {
    setupReader(""); // 空流
    ASSERT_THAT(reader->isEOF(), IsTrue());

    setupReader({0x01});
    uint8_t buf[1];
    reader->readBytes(buf, 1);
    ASSERT_THAT(reader->isEOF(), IsTrue());
}

