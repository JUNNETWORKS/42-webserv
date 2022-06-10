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

}  // namespace utils
