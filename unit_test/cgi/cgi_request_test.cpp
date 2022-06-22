#include "cgi/cgi_request.hpp"

#include <gtest/gtest.h>

#include "config/config.hpp"
#include "http/http_request.hpp"

namespace cgi {

TEST(CgiRequestTest, DefaultTest) {
  http::HttpRequest req;
  config::LocationConf location;

  std::string request_path = "/cgi-bin/test-cgi";
  cgi::CgiRequest cgi(request_path, req, location);

  EXPECT_EQ(request_path, cgi.GetCgiPath());
}

TEST(CgiRequestTest, QueryStringTest) {
  http::HttpRequest req;
  config::LocationConf location;

  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path_ = "/cgi-bin/test-cgi";
    std::string expect_query_string = "?";
    std::string request_path = expect_cgi_path_ + "?" + expect_query_string;
    cgi::CgiRequest cgi(request_path, req, location);

    EXPECT_EQ(expect_cgi_path_, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
}

}  // namespace cgi
