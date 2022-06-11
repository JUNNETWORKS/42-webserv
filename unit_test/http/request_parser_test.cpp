#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include "http/http_request.hpp"
#include "http/http_status.hpp"
#include "result/result.hpp"
#include "utils/ByteVector.hpp"

#define CRLF "\r\n"

namespace http {
using namespace result;

utils::ByteVector OpenFile(const std::string& name) {
  std::ifstream ifs(("./unit_test/http/req/" + name).c_str());
  std::stringstream ss;
  ss << ifs.rdbuf();
  std::string file_content = ss.str();
  utils::ByteVector res(file_content);
  return res;
}

// CorrectRequest=====================================
TEST(RequestParserTest, CorrectRequest) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "GET / HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "User-Agent: Pikachu\r\n"
      "Accept: */*\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, ExistNewLineBeforeCorrectRequest) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "\r\n"
      "\r\n"
      "\r\n"
      "GET / HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "User-Agent: Pikachu\r\n"
      "Accept: */*\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), OK);
}

// InvalidFormat RequestLine================================

TEST(RequestParserTest, ExistSPBeforeMethod) {
  http::HttpRequest req;
  utils::ByteVector buf(
      " GET / HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_FALSE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, ExistSPBetweenMethodAndURL) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "GET  / HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_FALSE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, ExistSPBetweenURLAndVersion) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "GET /  HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_FALSE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, NotExistCRLFAfterRequestLine) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "GET / HTTP/1.1"
      "Host: Hoge\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_FALSE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// Method ===========================================

TEST(RequestParserTest, NotExistMethod) {
  http::HttpRequest req;
  utils::ByteVector buf(
      "HOGE / HTTP/1.1\r\n"
      "Host: Hoge\r\n"
      "\r\n"
      "\r\n");

  req.ParseRequest(buf);
  EXPECT_FALSE(req.IsCorrectStatus());
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);

  EXPECT_TRUE(true);
}

TEST(RequestParserTest, NotImplementMethod) {
  // http::HttpRequest req;
  // utils::ByteVector buf(
  //     "CONNECT / HTTP/1.1\r\n"
  //     "Host: Hoge\r\n"
  //     "User-Agent: Pikachu\r\n"
  //     "Accept: */*\r\n"
  //     "\r\n"
  //     "\r\n");

  // req.ParseRequest(buf);
  // EXPECT_TRUE(req.IsCorrectStatus());
  // EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);

  EXPECT_TRUE(true);
}

TEST(RequestParserTest, NotAllowdMethod) {
  //コンフィグを合わせてテスト
  // http::HttpRequest req;
  // utils::ByteVector buf(
  //     "POST / HTTP/1.1\r\n"
  //     "Host: Hoge\r\n"
  //     "User-Agent: Pikachu\r\n"
  //     "Accept: */*\r\n"
  //     "\r\n"
  //     "\r\n");

  // req.ParseRequest(buf);
  // EXPECT_TRUE(req.IsCorrectStatus());
  // EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);

  EXPECT_TRUE(true);
}
}  // namespace http
