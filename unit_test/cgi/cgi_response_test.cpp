#include "cgi/cgi_response.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"
#include "utils/ByteVector.hpp"

namespace cgi {

typedef std::map<std::string, std::string> HeadersType;

// Status 付きのドキュメントレスポンス
// document-response = Content-Type [ Status ] *other-field NL response-body
TEST(CgiResponseParse, DocumentResponseWithStatus) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>404 Not Found</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  HeadersType expected_headers{{"CONTENT-TYPE", "text/html"},
                               {"STATUS", "404 Not Found"},
                               {"OPTIONAL", "hoge"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(),
            utils::ByteVector("<HTML>\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

// Status が付いていないドキュメントレスポンス
// Status がない場合は Status = 200 OK という扱いになる
TEST(CgiResponseParse, ValidDocumentResponseWithoutStatus) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>200 OK</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  HeadersType expected_headers{{"CONTENT-TYPE", "text/html"},
                               {"STATUS", "200 OK"},
                               {"OPTIONAL", "hoge"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(), utils::ByteVector("<HTML>\n"
                                                 "<body><p>200 OK</p></body>\n"
                                                 "</HTML>"));
}

// ローカルリダイレクト
// local-redir-response = local-Location NL
TEST(CgiResponseParse, ValidLocalRedirectResponse) {
  utils::ByteVector cgi_output(
      "Location:/users?q=jun\n"
      "\n");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kLocalRedirect);

  HeadersType expected_headers{{"LOCATION", "/users?q=jun"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);
}

// クライアントリダイレクト
// client-redir-response = client-Location *extension-field NL
TEST(CgiResponseParse, ValidClientRedirectResponse) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "ExtensionField: hoge\n"
      "\n");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kClientRedirect);

  HeadersType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"EXTENSIONFIELD", "hoge"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);
}

// ドキュメント付きクライアントリダイレクト
// client-redirdoc-response = client-Location Status Content-Type *other-field
// NL response-body
TEST(CgiResponseParse, ValidClientRedirectResponseWithDocument) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "Status: 302 Found\n"
      "Content-Type: text/html\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(),
            CgiResponse::kClientRedirectWithDocument);

  HeadersType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"STATUS", "302 Found"},
      {"CONTENT-TYPE", "text/html"},
      {"EXTENSIONFIELD", "hoge"},
      {"PROTOCOLFIELD", "fuga"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(),
            utils::ByteVector("<HTML>\n"
                              "<body><p>Let's redirect!!</p></body>\n"
                              "</HTML>"));
}

// ドキュメント付きクライアントリダイレクトの Status は､
// 302以外でもリダイレクトを表す3桁のHTTPステータスコードならOK
TEST(CgiResponseParse,
     ValidClientRedirectResponseWithDocumentAndCustomRedirectCode) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "Status: 301 Moved Permanently\n"
      "Content-Type: text/html\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  HeadersType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"STATUS", "301 Moved Permanently"},
      {"CONTENT-TYPE", "text/html"},
      {"EXTENSIONFIELD", "hoge"},
      {"PROTOCOLFIELD", "fuga"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(),
            utils::ByteVector("<HTML>\n"
                              "<body><p>Let's redirect!!</p></body>\n"
                              "</HTML>"));
}

// UNIX 上での CGI は改行コードとして CRLF も受け付け可能である
TEST(CgiResponseParse, NewLineIsCrlf) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\r\n"
      "Status: 404 Not Found\r\n"
      "Optional: hoge\r\n"
      "\r\n"
      "<HTML>\r\n"
      "<body><p>404 Not Found</p></body>\r\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  HeadersType expected_headers{{"CONTENT-TYPE", "text/html"},
                               {"STATUS", "404 Not Found"},
                               {"OPTIONAL", "hoge"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(),
            utils::ByteVector("<HTML>\r\n"
                              "<body><p>404 Not Found</p></body>\r\n"
                              "</HTML>"));
}

// body に LF と CRLF が混ざっているデータがあっても良い
TEST(CgiResponseParse, DocumentResponsesBodyIncludeLfAndCrlf) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\r\n"
      "<body><p>404 Not Found</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsOk());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  HeadersType expected_headers{{"CONTENT-TYPE", "text/html"},
                               {"STATUS", "404 Not Found"},
                               {"OPTIONAL", "hoge"}};
  EXPECT_EQ(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(cgi_res.GetBody(),
            utils::ByteVector("<HTML>\r\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

// ========================================================================
// Error case

// CRLF と LF が混ざっているものはエラー
TEST(CgiResponseParse, MixedNewLineIsError) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\r\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\r\n"
      "\n"
      "<HTML>\r"
      "<body><p>404 Not Found</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// レスポンスタイプ判別不能
TEST(CgiResponseParse, UnknownResponseType) {
  utils::ByteVector cgi_output(
      "Status: 200 OK\n"
      "\n"
      "<HTML>\n"
      "<body><p>200 OK</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ドキュメントレスポンスのStatusが不正
TEST(CgiResponseParse, DocumentResponseWithInvalidStatusIsError) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\n"
      "Status: 2000 Not Found\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>404 Not Found</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ローカルリダイレクトはLocation以外のフィールドを持てない
TEST(CgiResponseParse, LocalRedirectResponseWithOtherFields) {
  utils::ByteVector cgi_output(
      "Location:/users?q=jun\n"
      "OtherFiled: hoge\n"
      "\n");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ローカルリダイレクトは response-body を持てない
TEST(CgiResponseParse, LocalRedirectResponseWithBody) {
  utils::ByteVector cgi_output(
      "Location:/users?q=jun\n"
      "\n"
      "Local redirect can't hold body.");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// クライアントリダイレクトのURIの形式が間違っている
// URIの形式については https://tex2e.github.io/rfc-translater/html/rfc2396.html
// のセクション3を参照
TEST(CgiResponseParse, ClientRedirectResponseWithDocumentHasInvalidUri) {
  utils::ByteVector cgi_output(
      "Location:httpswww.google.com/search?q=pikachu\n"
      "Status: 302 Found\n"
      "Content-Type: text/html\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ドキュメント付きクライアントリダイレクトにStatusが無い
TEST(CgiResponseParse, ClientRedirectResponseWithDocumentWithoutStatus) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "Content-Type: text/html\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ドキュメント付きクライアントリダイレクトにContent-Typeが無い
TEST(CgiResponseParse, ClientRedirectResponseWithDocumentWioutContentType) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "Status: 302 Found\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

// ドキュメント付きクライアントリダイレクトで
// HTTPステータスコードがリダイレクトを表すステータスコードじゃない
TEST(CgiResponseParse, ClientRedirectResponseWithDocumentsStatusCodeIsInvalid) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "Status: 404 Not Found\n"
      "Content-Type: text/html\n"
      "ExtensionField: hoge\n"
      "ProtocolField: fuga\n"
      "\n"
      "<HTML>\n"
      "<body><p>Let's redirect!!</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_TRUE(cgi_res.Parse(cgi_output).IsErr());
}

}  // namespace cgi
