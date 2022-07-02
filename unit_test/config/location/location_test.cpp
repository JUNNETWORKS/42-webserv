#include <gtest/gtest.h>

#include <tuple>

#include "config/location_conf.hpp"

namespace config {

class LocationGetAbsTest
    : public ::testing::TestWithParam<
          std::tuple<std::string, std::string, std::string, std::string>> {};

TEST_P(LocationGetAbsTest, location_abs_test) {
  std::tuple<std::string, std::string, std::string, std::string> param =
      GetParam();
  std::string path_pattern = std::get<0>(param);
  std::string root_dir = std::get<1>(param);
  std::string req_path = std::get<2>(param);
  std::string expected = std::get<3>(param);

  LocationConf conf;
  conf.SetRootDir(root_dir);
  conf.SetPathPattern(path_pattern);

  EXPECT_EQ(conf.GetAbsolutePath(req_path), expected);
}

// {"location", "root_dir", "req_path", "expected"}
const std::vector<
    std::tuple<std::string, std::string, std::string, std::string>>
    LocationGetAbsTestTuple = {
        {"/", "/", "/", "/"},
        {"/", "/", "/hoge", "/hoge"},
        {"/", "/public", "/", "/public"},
        {"/", "/public", "/hoge", "/public/hoge"},
        {"/", "/public/", "/", "/public"},
        {"/", "/public/", "/hoge", "/public/hoge"},
        {"/cgi-bin", "/public/cgi-bin", "/cgi-bin/test-cgi",
         "/public/cgi-bin/test-cgi"},
        {"/cgi-bin/", "/public/cgi-bin", "/cgi-bin/test-cgi",
         "/public/cgi-bin/test-cgi"},
        {"/cgi-bin/", "/public/cgi-bin/", "/cgi-bin/test-cgi",
         "/public/cgi-bin/test-cgi"}};

// TODO : join path が 最後のパス消さないようにするべきか
// {"/", "/", "/hoge/", "/hoge/"}

INSTANTIATE_TEST_SUITE_P(LocationGetAbsTest, LocationGetAbsTest,
                         ::testing::ValuesIn(LocationGetAbsTestTuple));

}  // namespace config
