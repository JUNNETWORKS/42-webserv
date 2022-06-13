#include "utils/string.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"

namespace utils {

TEST(StoulTest, Zero) {
  EXPECT_RESULT_IS_OK(Stoul("0"));
  EXPECT_RESULT_OK_EQ(Stoul("0"), Result<unsigned long>(0));
}

TEST(StoulTest, Maximumvalue) {
  EXPECT_RESULT_IS_OK(Stoul("18446744073709551615"));
  EXPECT_RESULT_OK_EQ(Stoul("18446744073709551615"),
                      Result<unsigned long>(18446744073709551615ul));
}

TEST(StoulTest, MaximumvalueWithPlusSign) {
  EXPECT_RESULT_IS_ERR(Stoul("+18446744073709551615"));
}

TEST(StoulTest, MaximumvaluePlusOne) {
  EXPECT_RESULT_IS_ERR(Stoul("18446744073709551616"));
}

TEST(StoulTest, MaximumvalueWithPlusSigns) {
  EXPECT_RESULT_IS_ERR(Stoul("+++++++++++18446744073709551615"));
}

TEST(StoulTest, MinusOne) {
  EXPECT_RESULT_IS_ERR(Stoul("-1"));
}

TEST(StoulTest, MinusSigns) {
  EXPECT_RESULT_IS_ERR(Stoul("--------1"));
}

TEST(StoulTest, PlusZero) {
  EXPECT_RESULT_IS_ERR(Stoul("+0"));
}

TEST(StoulTest, MinusZero) {
  EXPECT_RESULT_IS_ERR(Stoul("-0"));
}

TEST(StoulTest, IncludeAlphabetFront) {
  EXPECT_RESULT_IS_ERR(Stoul("a100"));
}

TEST(StoulTest, IncludeAlphabetBack) {
  EXPECT_RESULT_IS_ERR(Stoul("100a"));
}

TEST(StoulTest, IncludeAlphabetMiddle) {
  EXPECT_RESULT_IS_ERR(Stoul("10a0"));
}

TEST(StoulTest, Hex_Zero) {
  EXPECT_RESULT_IS_OK(Stoul("0", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("0", kHexadecimal), Result<unsigned long>(0));
}

TEST(StoulTest, Hex_Maximumvalue) {
  EXPECT_RESULT_IS_OK(Stoul("FFFFFFFFFFFFFFFF", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("FFFFFFFFFFFFFFFF", kHexadecimal),
                      Result<unsigned long>(18446744073709551615ul));
}

TEST(StoulTest, Hex_OnlyCharValueLower) {
  EXPECT_RESULT_IS_OK(Stoul("abcdef", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("abcdef", kHexadecimal),
                      Result<unsigned long>(11259375));
}

TEST(StoulTest, Hex_OnlyCharValueUpper) {
  EXPECT_RESULT_IS_OK(Stoul("ABCDEF", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("ABCDEF", kHexadecimal),
                      Result<unsigned long>(11259375));
}

TEST(StoulTest, Hex_OnlyCharValueMix) {
  EXPECT_RESULT_IS_OK(Stoul("abcdefABCDEF", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("abcdefABCDEF", kHexadecimal),
                      Result<unsigned long>(188900977659375ul));
}

TEST(StoulTest, Hex_MixValue) {
  EXPECT_RESULT_IS_OK(Stoul("123BcD4F657e90A", kHexadecimal));
  EXPECT_RESULT_OK_EQ(Stoul("123BcD4F657e90A", kHexadecimal),
                      Result<unsigned long>(82116841074845962ul));
}

TEST(StoulTest, Hex_NonHexadecimalValue) {
  EXPECT_RESULT_IS_ERR(Stoul("100G", kHexadecimal));
}

TEST(StoulTest, Hex_ContainsMinusSign) {
  EXPECT_RESULT_IS_ERR(Stoul("-1aB", kHexadecimal));
}

TEST(StoulTest, Hex_ContainsPlusSign) {
  EXPECT_RESULT_IS_ERR(Stoul("+1aB", kHexadecimal));
}
}  // namespace utils
