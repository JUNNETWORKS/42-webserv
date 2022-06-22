#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"
#include "utils/string.hpp"

namespace utils {

// ENCODE
// ------------------------------------------------------------------------------------------
TEST(PercentEncodeTest, Japanese) {
  std::string s;
  std::string expect;

  s = "ほげ";
  expect = "%E3%81%BB%E3%81%92";

  EXPECT_RESULT_IS_OK(PercentEncode(s));
  EXPECT_RESULT_OK_EQ(PercentEncode(s), Result<std::string>(expect));

  s = "あいうえお";
  expect = "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
  EXPECT_RESULT_IS_OK(PercentEncode(s));
  EXPECT_RESULT_OK_EQ(PercentEncode(s), Result<std::string>(expect));
}

TEST(PercentEncodeTest, Isprint) {
  std::string s;
  for (int i = 0; i < 256; i++) {
    if (std::isprint(i) && i != '/' && i != ' ') {
      s += i;
    }
  }
  std::string expect =
      "%21%22%23%24%25%26%27%28%29%2A%2B%2C-.0123456789%3A%3B%3C%3D%3E%3F%"
      "40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%"
      "7B%7C%7D~";
  EXPECT_RESULT_IS_OK(PercentEncode(s));
  EXPECT_RESULT_OK_EQ(PercentEncode(s), Result<std::string>(expect));
}

// DECODE
// ------------------------------------------------------------------------------------------
TEST(PercentDecodeTest, Japanese) {
  std::string s;
  std::string expect;

  s = "%E3%81%BB%E3%81%92";
  expect = "ほげ";

  EXPECT_RESULT_IS_OK(PercentDecode(s));
  EXPECT_RESULT_OK_EQ(PercentDecode(s), Result<std::string>(expect));

  s = "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
  expect = "あいうえお";
  EXPECT_RESULT_IS_OK(PercentDecode(s));

  s = "あ%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
  expect = "あいうえお";
  EXPECT_RESULT_IS_OK(PercentDecode(s));
}

TEST(PercentDecodeTest, ErrCase) {
  std::string s;
  std::string expect;

  // s = "%E3%81%BB%E3%81%92";
  s = "%E3%81%BB%E3%81%9";
  // expect = "ほげ";

  EXPECT_RESULT_IS_ERR(PercentDecode(s));

  // s = "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A";
  s = "%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8";
  // expect = "あいうえお";
  EXPECT_RESULT_IS_ERR(PercentDecode(s));

  s = "%";
  EXPECT_RESULT_IS_ERR(PercentDecode(s));
  s = "%%";
  EXPECT_RESULT_IS_ERR(PercentDecode(s));
  s = "% 1";
  EXPECT_RESULT_IS_ERR(PercentDecode(s));
}

TEST(PercentDecodeTest, Isprint) {
  std::string expect;
  for (int i = 0; i < 256; i++) {
    if (std::isprint(i) && i != '/' && i != ' ') {
      expect += i;
    }
  }
  std::string s =
      "%21%22%23%24%25%26%27%28%29%2A%2B%2C-.0123456789%3A%3B%3C%3D%3E%3F%"
      "40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%"
      "7B%7C%7D~";
  EXPECT_RESULT_IS_OK(PercentDecode(s));
  EXPECT_RESULT_OK_EQ(PercentDecode(s), Result<std::string>(expect));
}

}  // namespace utils
