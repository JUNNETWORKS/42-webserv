#include "cgi/cgi_request.hpp"

#include <gtest/gtest.h>

#include "config/config.hpp"
#include "http/http_request.hpp"

namespace cgi {

TEST(CgiRequestTest, DefaultTest) {
  config::Config conf;
  config::LocationConf location;
  http::HttpRequest req(conf);

  std::string request_path = "/cgi-bin/test-cgi";
  cgi::CgiRequest cgi(request_path, req, location);
  cgi.ParseCigRequest();

  EXPECT_EQ(request_path, cgi.GetCgiPath());
}

// QueryStringTest
//  ------------------------------------------------------------------------------------------
TEST(CgiRequestTest, QueryStringTest) {
  config::Config conf;
  config::LocationConf location;
  http::HttpRequest req(conf);

  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "?";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
}

// ArgsTest
//  ------------------------------------------------------------------------------------------
TEST(CgiRequestTest, ArgsTest) {
  config::Config conf;
  config::LocationConf location;
  http::HttpRequest req(conf);

  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "";
    std::vector<std::string> expect_cgi_args = {};
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge+fuga";
    std::vector<std::string> expect_cgi_args = {"hoge", "fuga"};
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    // = があると argv を 設定しない
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge+fuga=hoge+fuga";
    std::vector<std::string> expect_cgi_args = {};
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "+";
    std::vector<std::string> expect_cgi_args = {"", ""};
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "++";
    std::vector<std::string> expect_cgi_args = {"", "", ""};
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);
    cgi.ParseCigRequest();

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
}

}  // namespace cgi
