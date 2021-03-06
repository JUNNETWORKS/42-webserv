#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"
#include "utils/string.hpp"

namespace utils {

// ENCODE
// ------------------------------------------------------------------------------------------
class EncodeTestOk
    : public ::testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(EncodeTestOk, Ok) {
  std::pair<std::string, std::string> prm = GetParam();
  std::string input = prm.first;
  std::string expected = prm.second;
  EXPECT_EQ(PercentEncode(input), expected);
}

const std::vector<std::pair<std::string, std::string>> EncodeOkVec = {
    {"あ", "%E3%81%82"},
    {"ほげ", "%E3%81%BB%E3%81%92"},
    {"ふが", "%E3%81%B5%E3%81%8C"},
    {"(; ;)", "%28%3B%20%3B%29"},
    {"あいうえお", "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A"},
    {"あiうeお", "%E3%81%82i%E3%81%86e%E3%81%8A"},
    {"kaきkuけko", "ka%E3%81%8Dku%E3%81%91ko"},
    {"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもや"
     "ゆよらりるれろわをん",
     "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A%E3%81%8B%E3%81%8D%E3%81%8F%"
     "E3%81%91%E3%81%93%E3%81%95%E3%81%97%E3%81%99%E3%81%9B%E3%81%9D%E3%81%9F%"
     "E3%81%A1%E3%81%A4%E3%81%A6%E3%81%A8%E3%81%AA%E3%81%AB%E3%81%AC%E3%81%AD%"
     "E3%81%AE%E3%81%AF%E3%81%B2%E3%81%B5%E3%81%B8%E3%81%BB%E3%81%BE%E3%81%BF%"
     "E3%82%80%E3%82%81%E3%82%82%E3%82%84%E3%82%86%E3%82%88%E3%82%89%E3%82%8A%"
     "E3%82%8B%E3%82%8C%E3%82%8D%E3%82%8F%E3%82%92%E3%82%93"}};

INSTANTIATE_TEST_SUITE_P(EncodeOk, EncodeTestOk,
                         ::testing::ValuesIn(EncodeOkVec));

// DECODE
// ------------------------------------------------------------------------------------------
class DecodeTestOk
    : public ::testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(DecodeTestOk, Ok) {
  std::pair<std::string, Result<std::string>> prm = GetParam();
  std::string input = prm.first;
  Result<std::string> expected = prm.second;
  EXPECT_RESULT_IS_OK(PercentDecode(input));
  EXPECT_RESULT_OK_EQ(PercentDecode(input), expected);
}

const std::vector<std::pair<std::string, std::string>> DecodeOkVec = {
    {"%E3%81%82", "あ"},
    {"%E3%81%BB%E3%81%92", "ほげ"},
    {"%E3%81%B5%E3%81%8C", "ふが"},
    {"%28%3B%20%3B%29", "(; ;)"},
    {"%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A", "あいうえお"},
    {"%E3%81%82i%E3%81%86e%E3%81%8A", "あiうeお"},
    {"ka%E3%81%8Dku%E3%81%91ko", "kaきkuけko"},
    {"%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A%E3%81%8B%E3%81%8D%E3%81%8F%"
     "E3%81%91%E3%81%93%E3%81%95%E3%81%97%E3%81%99%E3%81%9B%E3%81%9D%E3%81%9F%"
     "E3%81%A1%E3%81%A4%E3%81%A6%E3%81%A8%E3%81%AA%E3%81%AB%E3%81%AC%E3%81%AD%"
     "E3%81%AE%E3%81%AF%E3%81%B2%E3%81%B5%E3%81%B8%E3%81%BB%E3%81%BE%E3%81%BF%"
     "E3%82%80%E3%82%81%E3%82%82%E3%82%84%E3%82%86%E3%82%88%E3%82%89%E3%82%8A%"
     "E3%82%8B%E3%82%8C%E3%82%8D%E3%82%8F%E3%82%92%E3%82%93",
     "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもや"
     "ゆよらりるれろわをん"}};

INSTANTIATE_TEST_SUITE_P(DecodeOk, DecodeTestOk,
                         ::testing::ValuesIn(DecodeOkVec));

class DecodeTestErr : public ::testing::TestWithParam<std::string> {};

TEST_P(DecodeTestErr, Err) {
  std::string input = GetParam();

  EXPECT_RESULT_IS_ERR(PercentDecode(input));
}

const std::vector<std::string> DecodeErrVec = {"%",  "%%", "%%%", "% 01",
                                               "%1", "%E", "%E3%"};

INSTANTIATE_TEST_SUITE_P(DecodeErr, DecodeTestErr,
                         ::testing::ValuesIn(DecodeErrVec));

}  // namespace utils
