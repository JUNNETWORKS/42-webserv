#include "utils/string.hpp"

#include <gtest/gtest.h>

namespace utils {

TEST(StoulTest, Zero) {
  unsigned long num;
  EXPECT_TRUE(Stoul("0", num));
  EXPECT_EQ(num, 0);
}

TEST(StoulTest, Maximumvalue) {
  unsigned long num;
  EXPECT_TRUE(Stoul("18446744073709551615", num));
  EXPECT_EQ(num, 18446744073709551615ul);
}

TEST(StoulTest, MaximumvalueWithPlusSign) {
  unsigned long num;
  EXPECT_TRUE(Stoul("+18446744073709551615", num));
  EXPECT_EQ(num, 18446744073709551615ul);
}

TEST(StoulTest, MaximumvaluePlusOne) {
  unsigned long num;
  EXPECT_FALSE(Stoul("18446744073709551616", num));
}

TEST(StoulTest, MaximumvalueWithPlusSigns) {
  unsigned long num;
  EXPECT_FALSE(Stoul("+++++++++++18446744073709551615", num));
}

TEST(StoulTest, MinusOne) {
  unsigned long num;
  EXPECT_FALSE(Stoul("-1", num));
}

TEST(StoulTest, MinusSigns) {
  unsigned long num;
  EXPECT_FALSE(Stoul("--------1", num));
}

TEST(StoulTest, PlusZero) {
  unsigned long num;
  EXPECT_TRUE(Stoul("+0", num));
  EXPECT_EQ(num, 0);
}

TEST(StoulTest, MinusZero) {
  unsigned long num;
  EXPECT_FALSE(Stoul("-0", num));
}

TEST(StoulTest, IncludeAlphabetFront) {
  unsigned long num;
  EXPECT_FALSE(Stoul("a100", num));
}

TEST(StoulTest, IncludeAlphabetBack) {
  unsigned long num;
  EXPECT_FALSE(Stoul("100a", num));
}

TEST(StoulTest, IncludeAlphabetMiddle) {
  unsigned long num;
  EXPECT_FALSE(Stoul("10a0", num));
}

}  // namespace utils
