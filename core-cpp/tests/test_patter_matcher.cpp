//
// Created by user on 1/25/26.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <cstdint>
#include "gimt/gimt_patter_matcher.h"

using namespace testing;
using namespace gimt;

class PatternMatcherTest : public Test {
protected:
    // 准备通用的测试数据
    const uint8_t rawData[6] = {0x47, 0x49, 0x4D, 0x54, 0x01, 0x02}; // "GIMT" + \1\2
    const size_t rawLen = 6;
};

// 行为：当缓冲区以指定的字节序列开头时，应返回 true
TEST_F(PatternMatcherTest, ShouldReturnTrueWhenBufferMatchesByteSignature) {
    std::vector<uint8_t> signature = {0x47, 0x49, 0x4D, 0x54};

    bool result = PatternMatcher::match(rawData, rawLen, signature);

    ASSERT_THAT(result, IsTrue());
}

// 行为：当缓冲区完全等于签名时，应返回 true
TEST_F(PatternMatcherTest, ShouldReturnTrueWhenBufferExactlyMatchesSignature) {
    std::vector<uint8_t> signature = {0x47, 0x49, 0x4D, 0x54, 0x01, 0x02};

    ASSERT_THAT(PatternMatcher::match(rawData, rawLen, signature), IsTrue());
}

// 行为：当缓冲区长度小于签名长度时，即使前一部分匹配也应返回 false
TEST_F(PatternMatcherTest, ShouldReturnFalseWhenBufferIsShorterThanSignature) {
    std::vector<uint8_t> signature = {0x47, 0x49, 0x4D, 0x54, 0x01, 0x02, 0x03}; // 长度 7

    ASSERT_THAT(PatternMatcher::match(rawData, rawLen, signature), IsFalse());
}

// 行为：当缓冲区长度足够但内容不匹配时，应返回 false
TEST_F(PatternMatcherTest, ShouldReturnFalseWhenContentDoesNotMatch) {
    std::vector<uint8_t> signature = {0x47, 0x49, 0x58, 0x54}; // "GIXT"

    ASSERT_THAT(PatternMatcher::match(rawData, rawLen, signature), IsFalse());
}

// 行为：匹配字符串类型的签名
TEST_F(PatternMatcherTest, ShouldReturnTrueWhenBufferMatchesStringSignature) {
    std::string sigStr = "GIMT";

    ASSERT_THAT(PatternMatcher::matchString(rawData, rawLen, sigStr), IsTrue());
}

// 行为：字符串签名不匹配时返回 false
TEST_F(PatternMatcherTest, ShouldReturnFalseWhenStringSignatureMismatches) {
    ASSERT_THAT(PatternMatcher::matchString(rawData, rawLen, "JPEG"), IsFalse());
}

// 行为：空签名应被视为匹配（任何 buffer 都以 "空" 开头）
// 这是一个重要的边界行为确认
TEST_F(PatternMatcherTest, ShouldReturnTrueForEmptySignature) {
    ASSERT_THAT(PatternMatcher::match(rawData, rawLen, {}), IsTrue());
    ASSERT_THAT(PatternMatcher::matchString(rawData, rawLen, ""), IsTrue());
}

// 行为：空 Buffer 只能匹配空签名
TEST_F(PatternMatcherTest, ShouldHandleEmptyBuffer) {
    const uint8_t* emptyBuf = nullptr;
    ASSERT_THAT(PatternMatcher::match(emptyBuf, 0, {0x01}), IsFalse());
    ASSERT_THAT(PatternMatcher::match(emptyBuf, 0, {}), IsTrue());
}
