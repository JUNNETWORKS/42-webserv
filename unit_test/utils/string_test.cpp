#include "utils/string.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"

namespace utils {

//========================
// 成功のパターン

class StoulTestOk : public ::testing::TestWithParam<
                        std::pair<std::string, Result<unsigned long>>> {};

TEST_P(StoulTestOk, Ok) {
  std::pair<std::string, Result<unsigned long>> param = GetParam();
  std::string input = param.first;
  Result<unsigned long> expected = param.second;
  EXPECT_RESULT_IS_OK(Stoul(input));
  EXPECT_RESULT_OK_EQ(Stoul(input), expected);
}

const std::vector<std::pair<std::string, Result<unsigned long>>> StoulOkVec = {
    {"0", 0},                                          // 最小値
    {"18446744073709551615", 18446744073709551615ul},  // 最大値
};

INSTANTIATE_TEST_SUITE_P(StoulOk, StoulTestOk, ::testing::ValuesIn(StoulOkVec));

//========================
// エラーのパターン

class StoulTestNg : public ::testing::TestWithParam<std::string> {};

TEST_P(StoulTestNg, Ng) {
  std::string input = GetParam();
  // Result<unsigned long> expected = param.second;
  EXPECT_RESULT_IS_ERR(Stoul(input));
}

INSTANTIATE_TEST_SUITE_P(
    StoulNg, StoulTestNg,
    ::testing::Values("+18446744073709551615",  // +がある場合
                      "18446744073709551616",   // オーバーフロー
                      "+++++++++++18446744073709551615",  // +いっぱい
                      "-1",                               // マイナス
                      "--------1",  // マイナス符号いっぱい
                      "+0", "-0", "a100", "100a", "10a0"));

//========================
// 16進数成功のパターン

class StoulHexTestOk : public ::testing::TestWithParam<
                           std::pair<std::string, Result<unsigned long>>> {};

TEST_P(StoulHexTestOk, Ok) {
  std::pair<std::string, Result<unsigned long>> param = GetParam();
  std::string input = param.first;
  Result<unsigned long> expected = param.second;
  EXPECT_RESULT_IS_OK(Stoul(input, kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul(input, kHexadecimal), expected);
}

const std::vector<std::pair<std::string, Result<unsigned long>>> StoulHexOkVec =
    {{"0", 0},
     {"FFFFFFFFFFFFFFFF", 18446744073709551615ul},
     {"abcdef", 11259375},
     {"ABCDEF", 11259375},
     {"abcdefABCDEF", 188900977659375ul},
     {"123BcD4F657e90A", 82116841074845962ul}};

INSTANTIATE_TEST_SUITE_P(StoulHexTestOk, StoulHexTestOk,
                         ::testing::ValuesIn(StoulHexOkVec));

//========================
// 16進数エラーのパターン

class StoulHexTestNg : public ::testing::TestWithParam<std::string> {};

TEST_P(StoulHexTestNg, Ng) {
  std::string input = GetParam();
  // Result<unsigned long> expected = param.second;
  EXPECT_RESULT_IS_ERR(Stoul(input));
}

INSTANTIATE_TEST_SUITE_P(StoulHexTestNg, StoulHexTestNg,
                         ::testing::Values("100G",  // Hex_NonHexadecimalValue
                                           "-1aB",  // Hex_ContainsMinusSign
                                           "+1aB"   // Hex_ContainsPlusSign
                                           ));

// Split Test
// ------------------------------------------------------------------------------------------
class SplitTest
    : public ::testing::TestWithParam<std::pair<
          std::pair<std::string, std::string>, std::vector<std::string>>> {};

TEST_P(SplitTest, Ok) {
  std::pair<std::pair<std::string, std::string>, std::vector<std::string>>
      param = GetParam();
  std::pair<std::string, std::string> input = param.first;
  std::vector<std::string> expected = param.second;

  EXPECT_EQ(SplitString(input.first, input.second), expected);
}

const std::vector<
    std::pair<std::pair<std::string, std::string>, std::vector<std::string>>>
    SplitTestCase = {
        {{"", ""}, {""}},
        {{"a", ""}, {"a"}},
        {{"", " "}, {""}},
        {{" ", " "}, {"", ""}},
        {{"   ", " "}, {"", "", "", ""}},
        {{"a b c", " "}, {"a", "b", "c"}},
        {{" a b c", " "}, {"", "a", "b", "c"}},
        {{" a b c ", " "}, {"", "a", "b", "c", ""}},
        {{"--a---b---c--", "---"}, {"--a", "b", "c--"}},
        {{"-a-", "-"}, {"", "a", ""}},
        {{"-a-", "---"}, {"-a-"}},
        {{"-a-", "-----"}, {"-a-"}},
        {{"---a---", "-"}, {"", "", "", "a", "", "", ""}},
        {{"---a---", "---"}, {"", "a", ""}},
};

INSTANTIATE_TEST_SUITE_P(SplitTest, SplitTest,
                         ::testing::ValuesIn(SplitTestCase));

}  // namespace utils
