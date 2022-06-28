#include "utils/path.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"

namespace utils {

// Path Join
// ------------------------------------------------------------------------------------------
class PathTestOk
    : public ::testing::TestWithParam<
          std::pair<std::pair<std::string, std::string>, std::string>> {};

TEST_P(PathTestOk, Ok) {
  std::pair<std::pair<std::string, std::string>, std::string> param =
      GetParam();
  std::pair<std::string, std::string> input = param.first;
  std::string expected = param.second;
  EXPECT_EQ(PathJoin(input.first, input.second), expected);
}

const std::vector<std::pair<std::pair<std::string, std::string>, std::string>>
    PathJoinVec = {
        {{"a", "b"}, "a/b"},
        {{"a", "/b"}, "a/b"},
        {{"a/", "/b"}, "a/b"},
        {{"/a", "/b"}, "/a/b"},
        {{"/a/", "/b/"}, "/a/b"},
        {{"a/b/c", "x/y/z"}, "a/b/c/x/y/z"},
        {{"a///b///c", "x/./y/./z"}, "a/b/c/x/y/z"},
        {{"///a///", "///b///"}, "/a/b"},
        {{"a/b/c", "d/e/f"}, "a/b/c/d/e/f"},
        {{"/a/b/c/", "/d/e/f"}, "/a/b/c/d/e/f"},
        {{"///a///b///c///", "///d///e///f///"}, "/a/b/c/d/e/f"},
};

INSTANTIATE_TEST_SUITE_P(PathJoin, PathTestOk,
                         ::testing::ValuesIn(PathJoinVec));

//  Path Normalization
// ------------------------------------------------------------------------------------------
class PathNormalizationTestOk
    : public ::testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(PathNormalizationTestOk, Ok) {
  std::pair<std::string, std::string> param = GetParam();
  std::string input = param.first;
  Result<std::string> expected = param.second;
  EXPECT_TRUE(IsValidPath(input));
  EXPECT_RESULT_IS_OK(PathNormalization(input));
  EXPECT_RESULT_OK_EQ(PathNormalization(input), expected);
}

const std::vector<std::pair<std::string, std::string>> PathNormalizationOkVec =
    {
        {"/hoge/fuga", "/hoge/fuga"},
        {"/hoge///fuga", "/hoge/fuga"},
        {"/hoge/./././", "/hoge"},
        {"/hoge/..", "/"},
        {"/hoge/../", "/"},
        {"/hoge/fuga/..", "/hoge"},
        {"/hoge/fuga/../", "/hoge"},
        {"/hoge/abc/../xyz/..", "/hoge"},
        {"/hoge/fuga/./././", "/hoge/fuga"},
        {"/hoge/fuga/./.././", "/hoge"},
};

INSTANTIATE_TEST_SUITE_P(PathNormalization, PathNormalizationTestOk,
                         ::testing::ValuesIn(PathNormalizationOkVec));

//
class PathNormalizationTestErr : public ::testing::TestWithParam<std::string> {
};

TEST_P(PathNormalizationTestErr, Ok) {
  std::string param = GetParam();
  std::string input = param;
  EXPECT_FALSE(IsValidPath(input));
  EXPECT_RESULT_IS_ERR(PathNormalization(input));
}

const std::vector<std::string> PathNormalizationErrVec = {
    "/..",      "/../",        "///../",       "/..///",
    "///..///", "/hoge/../..", "/hoge/../../", "/hoge/fuga/../../..",
};

INSTANTIATE_TEST_SUITE_P(PathNormalizationErr, PathNormalizationTestErr,
                         ::testing::ValuesIn(PathNormalizationErrVec));

}  // namespace utils
