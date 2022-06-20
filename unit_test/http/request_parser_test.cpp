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
const std::string kConfigurationDirPath = "unit_test/http/config/";
const config::Config default_conf =
    config::ParseConfig((kConfigurationDirPath + "SimpleServer.conf").c_str());

utils::ByteVector OpenFile(const std::string& name) {
  std::ifstream ifs(("./unit_test/http/req/" + name).c_str());
  std::stringstream ss;
  ss << ifs.rdbuf();
  std::string file_content = ss.str();
  utils::ByteVector res(file_content);
  return res;
}

TEST(RequestParserTest, KOFormatExistOBSfold) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatExistOBSfold.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPAfterVersion) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatExistSPAfterVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBeforeSpace) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatExistSPBeforeSpace.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBetWeenMethodAndURL) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatExistSPBetWeenMethodAndURL.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatExistSPBetWeenURLAndVersion) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatExistSPBetWeenURLAndVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistCRLF) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatNotExistCRLF.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOFormatNotExistHostHeader) {
//   http::HttpRequest req(default_conf, 8080);
//   utils::ByteVector buf = OpenFile("KOFormatNotExistHostHeader.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
// }

TEST(RequestParserTest, KOFormatNotExistMethod) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatNotExistMethod.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistRequestLine) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatNotExistRequestLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistURL) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatNotExistURL.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFormatNotExistVersion) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFormatNotExistVersion.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderExistSPBeforeColon) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOHeaderExistSPBeforeColon.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderExistTabBeforeColon) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOHeaderExistTabBeforeColon.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOHeaderNotExistDquotePair) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOHeaderNotExistDquotePair.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOMethodNotAllowd) {
//   http::HttpRequest req(default_conf, 8080);
//   utils::ByteVector buf = OpenFile("KOMethodNotAllowd.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), NOT_ALLOWED);
// }

// TEST(RequestParserTest, KOMethodNotImplemented) {
//   http::HttpRequest req(default_conf, 8080);
//   utils::ByteVector buf = OpenFile("KOMethodNotImplemented.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);
// }

TEST(RequestParserTest, KOUnknownMethod) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOUnknownMethod.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);
}

TEST(RequestParserTest, KOURLTooLong) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOURLTooLong.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), URI_TOO_LONG);
}

TEST(RequestParserTest, KOVersioExistMultipleDot) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersioExistMultipleDot.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersioInvalidMinorLong) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersioInvalidMinorLong.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersionInvalidMajorLong) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersionInvalidMajorLong.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersionInvalidMajorLower) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersionInvalidMajorLower.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

// TEST(RequestParserTest, KOVersionInvalidMajorUpper) {
//   http::HttpRequest req(default_conf, 8080);
//   utils::ByteVector buf = OpenFile("KOVersionInvalidMajorUpper.txt");

//   req.ParseRequest(buf);
//   EXPECT_TRUE(req.IsCorrectStatus() == false);
//   EXPECT_EQ(req.GetParseStatus(), HTTP_VERSION_NOT_SUPPORTED);
// }

TEST(RequestParserTest, KOVersionInvalidPrefix) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersionInvalidPrefix.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOVersioNotExistDot) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOVersioNotExistDot.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, OKCommaInDquote) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKCommaInDquote.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKCorrectNewLine) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKCorrectNewLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKCorrect) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKCorrect.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderDquoteStringEscape) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderDquoteStringEscape.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderDquoteString) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderDquoteString.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderExistOWSBeforeValue) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderExistOWSBeforeValue.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderExistOWSftrerValue) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderExistOWSftrerValue.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderListMultipleLine) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderListMultipleLine.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKHeaderList) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKHeaderList.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, OKVersionMinorUpper) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKVersionMinorUpper.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);
}

TEST(RequestParserTest, KOBodyChunkSizeTooLarge) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOBodyChunkSizeTooLarge.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), PAYLOAD_TOO_LARGE);
}

TEST(RequestParserTest, KOBodyInvalidChunkSizeLower) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOBodyInvalidChunkSizeLower.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOBodyInvalidChunkSizeUpper) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOBodyInvalidChunkSizeUpper.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOBodyNotExistChunkSize) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOBodyNotExistChunkSize.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), BAD_REQUEST);
}

TEST(RequestParserTest, KOFieldInvalidTransferEncoding) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("KOFieldInvalidTransferEncoding.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == false);
  EXPECT_EQ(req.GetParseStatus(), NOT_IMPLEMENTED);
}

TEST(RequestParserTest, OKBodyChunkSizeZero) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKBodyChunkSizeZero.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);

  const utils::ByteVector expect_body("");
  const utils::ByteVector body = req.GetBody();
  EXPECT_EQ(body, expect_body);
}

TEST(RequestParserTest, OKBodyCorrectChunkHexadecimal) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKBodyCorrectChunkHexadecimal.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);

  const utils::ByteVector expect_body(
      "123456789012345678901234567890123456789012345678901234567890123456789012"
      "345678901234567890123456789012345678901234567890123456789012345678901234"
      "567890123456789012345678901123456789012123456789012312345678901234123456"
      "789012345");
  const utils::ByteVector body = req.GetBody();
  EXPECT_EQ(body, expect_body);
}

TEST(RequestParserTest, OKBodyCorrectChunk) {
  http::HttpRequest req(default_conf, 8080);
  utils::ByteVector buf = OpenFile("OKBodyCorrectChunk.txt");

  req.ParseRequest(buf);
  EXPECT_TRUE(req.IsCorrectStatus() == true);
  EXPECT_EQ(req.GetParseStatus(), OK);

  const utils::ByteVector expect_body("12345abcde");
  const utils::ByteVector body = req.GetBody();
  EXPECT_EQ(body, expect_body);
}

}  // namespace http
