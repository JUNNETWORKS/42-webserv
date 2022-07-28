#include "cgi/cgi_response.hpp"

#include <gtest/gtest.h>

#include "expectations/expect_result.hpp"
#include "utils/ByteVector.hpp"

namespace cgi {

utils::ByteVector DecodeChunkResponse(utils::ByteVector data) {
  std::string decoded;

  int n;
  while (data.empty() == false) {
    Result<size_t> pos = data.FindString(http::kCrlf);
    std::string line = data.SubstrBeforePos(pos.Ok());
    Result<unsigned long> stoul_res = utils::Stoul(line, utils::kHexadecimal);
    n = stoul_res.Ok();
    if (n == 0) {
      break;
    }
    data.EraseHead(line.length());
    data.EraseHead(http::kCrlf.length());  // crlf
    decoded += data.SubstrBeforePos(stoul_res.Ok());
    data.EraseHead(stoul_res.Ok());
    data.EraseHead(http::kCrlf.length());  // crlf
  }
  EXPECT_EQ(n, 0);
  return decoded;
}

// HeaderVecType
// はベクターなので一度Mapに変換することによって順番によるFAILが発生しないようにする
void EXPECT_EQ_HEADERS(const CgiResponse::HeaderVecType &actual,
                       const CgiResponse::HeaderVecType &expected) {
  std::map<std::string, std::string> actual_map(actual.begin(), actual.end());
  std::map<std::string, std::string> expected_map(expected.begin(),
                                                  expected.end());
  EXPECT_EQ(actual_map, expected_map);
}

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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kDocumentResponse);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);

  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kDocumentResponse);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "200 OK"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);

  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kLocalRedirect);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kLocalRedirect);

  CgiResponse::HeaderVecType expected_headers{{"LOCATION", "/users?q=jun"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
}

// クライアントリダイレクト
// client-redir-response = client-Location *extension-field NL
TEST(CgiResponseParse, ValidClientRedirectResponse) {
  utils::ByteVector cgi_output(
      "Location:https://www.google.com/search?q=pikachu\n"
      "ExtensionField: hoge\n"
      "\n");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kClientRedirect);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kClientRedirect);

  CgiResponse::HeaderVecType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"EXTENSIONFIELD", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output),
            CgiResponse::kClientRedirectWithDocument);

  EXPECT_EQ(cgi_res.GetResponseType(),
            CgiResponse::kClientRedirectWithDocument);

  CgiResponse::HeaderVecType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"STATUS", "302 Found"},
      {"CONTENT-TYPE", "text/html"},
      {"EXTENSIONFIELD", "hoge"},
      {"PROTOCOLFIELD", "fuga"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
                              "<body><p>Let's redirect!!</p></body>\n"
                              "</HTML>"));
}

// ドキュメント付きクライアントリダイレクトの Status は､
// 302以外でもリダイレクトを表す300番台のHTTPステータスコードならOK
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
  EXPECT_EQ(cgi_res.Parse(cgi_output),
            CgiResponse::kClientRedirectWithDocument);

  CgiResponse::HeaderVecType expected_headers{
      {"LOCATION", "https://www.google.com/search?q=pikachu"},
      {"STATUS", "301 Moved Permanently"},
      {"CONTENT-TYPE", "text/html"},
      {"EXTENSIONFIELD", "hoge"},
      {"PROTOCOLFIELD", "fuga"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kDocumentResponse);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kDocumentResponse);

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\r\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

TEST(CgiResponseParse, ChoppedBuffer) {
  CgiResponse cgi_res;

  utils::ByteVector buffer;

  const char *cgi_output1(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: ho");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output1),
                            strlen(cgi_output1));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kNotIdentified);

  const char *cgi_output2(
      "ge\n"
      "\n"
      "<HTML>\n"
      "<body><p>4");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output2),
                            strlen(cgi_output2));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  const char *cgi_output3(
      "04 Not Found</p></body>\n"
      "</HTML>");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output3),
                            strlen(cgi_output3));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  // Parse後にbufferにデータは残っていないはず
  EXPECT_TRUE(buffer.empty());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

// buffer の切れ目がヘッダーとボディのの区切りの手前
TEST(CgiResponseParse, ChoppedBufferThatIsSplitedBeforeHeaderBoundary) {
  CgiResponse cgi_res;

  utils::ByteVector buffer;

  const char *cgi_output1(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\n\n");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output1),
                            strlen(cgi_output1));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  const char *cgi_output2(
      "<HTML>\n"
      "<body><p>4");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output2),
                            strlen(cgi_output2));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  const char *cgi_output3(
      "04 Not Found</p></body>\n"
      "</HTML>");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output3),
                            strlen(cgi_output3));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  // Parse後にbufferにデータは残っていないはず
  EXPECT_TRUE(buffer.empty());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

// buffer の切れ目がヘッダーとボディのの区切りの間
TEST(CgiResponseParse, ChoppedBufferThatIsSplitedInMiddleHeaderBoundary) {
  CgiResponse cgi_res;

  utils::ByteVector buffer;

  const char *cgi_output1(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\n");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output1),
                            strlen(cgi_output1));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kNotIdentified);

  const char *cgi_output2(
      "\n"
      "<HTML>\n"
      "<body><p>4");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output2),
                            strlen(cgi_output2));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  const char *cgi_output3(
      "04 Not Found</p></body>\n"
      "</HTML>");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output3),
                            strlen(cgi_output3));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  // Parse後にbufferにデータは残っていないはず
  EXPECT_TRUE(buffer.empty());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
                              "<body><p>404 Not Found</p></body>\n"
                              "</HTML>"));
}

// buffer の切れ目がヘッダーとボディのの区切りの後
TEST(CgiResponseParse, ChoppedBufferThatIsSplitedAftertHeaderBoundary) {
  CgiResponse cgi_res;

  utils::ByteVector buffer;

  const char *cgi_output1(
      "Content-Type: text/html\n"
      "Status: 404 Not Found\n"
      "Optional: hoge");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output1),
                            strlen(cgi_output1));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kNotIdentified);

  const char *cgi_output2(
      "\n"
      "\n"
      "<HTML>\n"
      "<body><p>4");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output2),
                            strlen(cgi_output2));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  const char *cgi_output3(
      "04 Not Found</p></body>\n"
      "</HTML>");
  buffer.AppendDataToBuffer(reinterpret_cast<const utils::Byte *>(cgi_output3),
                            strlen(cgi_output3));
  EXPECT_EQ(cgi_res.Parse(buffer), CgiResponse::kDocumentResponse);

  // Parse後にbufferにデータは残っていないはず
  EXPECT_TRUE(buffer.empty());

  EXPECT_EQ(cgi_res.GetResponseType(), CgiResponse::kDocumentResponse);

  CgiResponse::HeaderVecType expected_headers{{"CONTENT-TYPE", "text/html"},
                                              {"STATUS", "404 Not Found"},
                                              {"OPTIONAL", "hoge"}};
  EXPECT_EQ_HEADERS(cgi_res.GetHeaders(), expected_headers);
  cgi_res.AppendLastChunk();
  EXPECT_EQ(DecodeChunkResponse(cgi_res.GetBody()),
            utils::ByteVector("<HTML>\n"
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

// ヘッダー名が無い
TEST(CgiResponseParse, HeaderNameIsNone) {
  utils::ByteVector cgi_output(
      ": text/html\n"
      "Status: 200 OK\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>200 OK</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

// ヘッダーにコロンが無い
TEST(CgiResponseParse, NoColonAfterHeaderName) {
  utils::ByteVector cgi_output(
      "Content-Type text/html\n"
      "Status: 200 OK\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>200 OK</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

// 同じ名前のヘッダーが2つある
TEST(CgiResponseParse, HeaderDuplication) {
  utils::ByteVector cgi_output(
      "Content-Type: text/html\n"
      "Status: 200 OK\n"
      "Status: 404 Not Found\n"
      "Optional: hoge\n"
      "\n"
      "<HTML>\n"
      "<body><p>200 OK</p></body>\n"
      "</HTML>");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

// ローカルリダイレクトはLocation以外のフィールドを持てない
TEST(CgiResponseParse, LocalRedirectResponseWithOtherFields) {
  utils::ByteVector cgi_output(
      "Location:/users?q=jun\n"
      "OtherFiled: hoge\n"
      "\n");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

// ローカルリダイレクトは response-body を持てない
TEST(CgiResponseParse, LocalRedirectResponseWithBody) {
  utils::ByteVector cgi_output(
      "Location:/users?q=jun\n"
      "\n"
      "Local redirect can't hold body.");

  CgiResponse cgi_res;
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
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
  EXPECT_EQ(cgi_res.Parse(cgi_output), CgiResponse::kParseError);
}

}  // namespace cgi
