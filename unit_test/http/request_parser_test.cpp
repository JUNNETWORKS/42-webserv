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

TEST(RequestParserTest, KOFormatExistOBSfold) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatExistOBSfold.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPAfterVersion) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatExistSPAfterVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBeforeSpace) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatExistSPBeforeSpace.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBetWeenMethodAndURL) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatExistSPBetWeenMethodAndURL.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBetWeenURLAndVersion) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatExistSPBetWeenURLAndVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistCRLF) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatNotExistCRLF.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOFormatNotExistHostHeader) {
//   http::HttpRequest req;
//   utils::ByteVector buf = OpenFile("KOFormatNotExistHostHeader.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
// }

TEST(RequestParserTest, KOFormatNotExistMethod) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatNotExistMethod.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistRequestLine) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatNotExistRequestLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistURL) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatNotExistURL.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistVersion) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOFormatNotExistVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderExistSPBeforeColon) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOHeaderExistSPBeforeColon.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderExistTabBeforeColon) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOHeaderExistTabBeforeColon.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderNotExistDquotePair) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOHeaderNotExistDquotePair.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOMethodNotAllowd) {
//   http::HttpRequest req;
//   utils::ByteVector buf = OpenFile("KOMethodNotAllowd.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), NOT_ALLOWED);
// }

// TEST(RequestParserTest, KOMethodNotImplemented) {
//   http::HttpRequest req;
//   utils::ByteVector buf = OpenFile("KOMethodNotImplemented.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);
// }

TEST(RequestParserTest, KOUnknownMethod) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOUnknownMethod.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOURLTooLong) {
//   http::HttpRequest req;
//   utils::ByteVector buf = OpenFile("KOURLTooLong.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), URI_TOO_LONG);
// }

TEST(RequestParserTest, KOVersioExistMultipleDot) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersioExistMultipleDot.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersioInvalidMinorLong) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersioInvalidMinorLong.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersionInvalidMajorLong) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersionInvalidMajorLong.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersionInvalidMajorLower) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersionInvalidMajorLower.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOVersionInvalidMajorUpper) {
//   http::HttpRequest req;
//   utils::ByteVector buf = OpenFile("KOVersionInvalidMajorUpper.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), HTTP_VERSION_NOT_SUPPORTED);
// }

TEST(RequestParserTest, KOVersionInvalidPrefix) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersionInvalidPrefix.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersioNotExistDot) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("KOVersioNotExistDot.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, OKCommaInDquote) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKCommaInDquote.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKCorrectNewLine) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKCorrectNewLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKCorrect) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKCorrect.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderDquoteStringEscape) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderDquoteStringEscape.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderDquoteString) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderDquoteString.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderExistOWSBeforeValue) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderExistOWSBeforeValue.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderExistOWSftrerValue) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderExistOWSftrerValue.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderListMultipleLine) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderListMultipleLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderList) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKHeaderList.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKVersionMinorUpper) {
  http::HttpRequest req;
  utils::ByteVector buf = OpenFile("OKVersionMinorUpper.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

}  // namespace http
