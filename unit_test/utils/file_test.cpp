#include "utils/File.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"

namespace utils {

static void test_all_file_operator(const std::string &s1,
                                   const std::string &s2) {
  EXPECT_TRUE((File(s1) < File(s2)) == (s1 < s2));
  EXPECT_TRUE((File(s1) <= File(s2)) == (s1 <= s2));
  EXPECT_TRUE((File(s1) > File(s2)) == (s1 > s2));
  EXPECT_TRUE((File(s1) >= File(s2)) == (s1 >= s2));

  EXPECT_TRUE((File(s2) < File(s1)) == (s2 < s1));
  EXPECT_TRUE((File(s2) <= File(s1)) == (s2 <= s1));
  EXPECT_TRUE((File(s2) > File(s1)) == (s2 > s1));
  EXPECT_TRUE((File(s2) >= File(s1)) == (s2 >= s1));
}

TEST(FILE, OPERATOR1) {
  EXPECT_FALSE(File("abc") > File("xyz"));
  EXPECT_FALSE(File("abc") > File("abc"));
  EXPECT_TRUE(File("xyz") > File("abc"));
}

TEST(FILE, OPERATOR2) {
  EXPECT_FALSE(File("abc") >= File("xyz"));
  EXPECT_TRUE(File("abc") >= File("abc"));
  EXPECT_TRUE(File("xyz") >= File("abc"));
}

TEST(FILE, OPERATOR3) {
  EXPECT_TRUE(File("abc") < File("xyz"));
  EXPECT_FALSE(File("abc") < File("abc"));
  EXPECT_FALSE(File("xyz") < File("abc"));
}

TEST(FILE, OPERATOR4) {
  EXPECT_TRUE(File("abc") <= File("xyz"));
  EXPECT_TRUE(File("abc") <= File("abc"));
  EXPECT_FALSE(File("xyz") <= File("abc"));
}

TEST(FILE, ALL) {
  test_all_file_operator("hoge", "fuga");
  test_all_file_operator("hoge", "HOGE");
  test_all_file_operator("aaa", "xxx");
}

}  // namespace utils
