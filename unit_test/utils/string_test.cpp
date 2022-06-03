#include "utils/string.hpp"

#include <gtest/gtest.h>

namespace utils {

TEST(StoulTest, Zero) {
  unsigned long num;
  EXPECT_TRUE(Stoul(num, "0"));
  EXPECT_EQ(num, 0);
}

TEST(StoulTest, Maximumvalue) {
  unsigned long num;
  EXPECT_TRUE(Stoul(num, "18446744073709551615"));
  EXPECT_EQ(num, 18446744073709551615ul);
}

TEST(StoulTest, MaximumvalueWithPlusSign) {
  unsigned long num;
  EXPECT_TRUE(Stoul(num, "+18446744073709551615"));
  EXPECT_EQ(num, 18446744073709551615ul);
}

TEST(StoulTest, MaximumvaluePlusOne) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "18446744073709551616"));
}

TEST(StoulTest, MaximumvalueWithPlusSigns) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "+++++++++++18446744073709551615"));
}

TEST(StoulTest, MinusOne) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "-1"));
}

TEST(StoulTest, MinusSigns) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "--------1"));
}

TEST(StoulTest, PlusZero) {
  unsigned long num;
  EXPECT_TRUE(Stoul(num, "+0"));
  EXPECT_EQ(num, 0);
}

TEST(StoulTest, MinusZero) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "-0"));
}

TEST(StoulTest, IncludeAlphabetFront) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "a100"));
}

TEST(StoulTest, IncludeAlphabetBack) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "100a"));
}

TEST(StoulTest, IncludeAlphabetMiddle) {
  unsigned long num;
  EXPECT_FALSE(Stoul(num, "10a0"));
}

}  // namespace utils
