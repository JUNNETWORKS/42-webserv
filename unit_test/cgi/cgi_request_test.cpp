#include "cgi/cgi_request.hpp"

#include <gtest/gtest.h>

#include "config/config.hpp"
#include "http/http_request.hpp"
#include "server/socket.hpp"
#include "server/socket_address.hpp"

namespace cgi {

TEST(CgiRequestTest, DefaultTest) {
  config::Config conf;
  server::ConnSocket sample_sock(-1, server::SocketAddress(),
                                 server::SocketAddress(), conf);
  config::LocationConf location;
  http::HttpRequest req;

  std::string expect_cgi_path = "/cgi-bin/test-cgi";
  req.SetPath(expect_cgi_path);
  cgi::CgiRequest cgi(&sample_sock, req, location);
  cgi.ParseCgiRequest();

  EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
}

// QueryStringTest
//  ------------------------------------------------------------------------------------------
TEST(CgiRequestTest, QueryStringTest) {
  config::Config conf;
  server::ConnSocket sample_sock(-1, server::SocketAddress(),
                                 server::SocketAddress(), conf);
  config::LocationConf location;
  http::HttpRequest req;

  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge";
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "";
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "?";
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
  }
}

// ArgsTest
//  ------------------------------------------------------------------------------------------
TEST(CgiRequestTest, ArgsTest) {
  config::Config conf;
  server::ConnSocket sample_sock(-1, server::SocketAddress(),
                                 server::SocketAddress(), conf);
  config::LocationConf location;
  http::HttpRequest req;

  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "";
    std::vector<std::string> expect_cgi_args = {};
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge+fuga";
    std::vector<std::string> expect_cgi_args = {"hoge", "fuga"};
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    // = があると argv を 設定しない
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "hoge+fuga=hoge+fuga";
    std::vector<std::string> expect_cgi_args = {};
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "+";
    std::vector<std::string> expect_cgi_args = {"", ""};
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
  {
    std::string expect_cgi_path = "/cgi-bin/test-cgi";
    std::string expect_query_string = "++";
    std::vector<std::string> expect_cgi_args = {"", "", ""};
    std::string request_path = expect_cgi_path + "?" + expect_query_string;
    req.SetPath(expect_cgi_path);
    req.SetQueryParam(expect_query_string);
    cgi::CgiRequest cgi(&sample_sock, req, location);
    cgi.ParseCgiRequest();

    EXPECT_EQ(expect_cgi_path, cgi.GetCgiPath());
    EXPECT_EQ(expect_query_string, cgi.GetQueryString());
    EXPECT_EQ(expect_cgi_args, cgi.GetCgiArgs());
  }
}

}  // namespace cgi
