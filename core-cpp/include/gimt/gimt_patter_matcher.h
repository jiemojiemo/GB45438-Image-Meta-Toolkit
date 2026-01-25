//
// Created by user on 1/25/26.
//

#ifndef GIMT_PATTER_MATCHER_H
#define GIMT_PATTER_MATCHER_H
#include <string>
namespace gimt {
class PatternMatcher {
public:
  // 检查 buffer 是否以指定的签名开头
  static bool match(const uint8_t* buffer, size_t bufferLen, const std::vector<uint8_t>& signature) {
    if (bufferLen < signature.size()) return false;
    return std::equal(signature.begin(), signature.end(), buffer);
  }

  // 方便函数：匹配字符串类型的签名
  static bool matchString(const uint8_t* buffer, size_t bufferLen, const std::string& sigStr) {
    if (bufferLen < sigStr.size()) return false;
    return std::equal(sigStr.begin(), sigStr.end(), buffer);
  }
};
}

#endif //GIMT_PATTER_MATCHER_H
