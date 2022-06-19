#ifndef CGI_CGI_RESPONSE_HPP_
#define CGI_CGI_RESPONSE_HPP_

#include <map>
#include <string>

#include "http/http_response.hpp"
#include "result/result.hpp"
#include "utils/ByteVector.hpp"

namespace cgi {
using namespace result;

const std::string kLF = "\n";
const std::string kCRLF = "\r\n";

class CgiResponse {
 public:
  enum ResponseType {
    kNotIdentified,
    kDocumentResponse,
    kLocalRedirect,
    kClientRedirect,
    kClientRedirectWithDocument,
    kParseError
  };

 private:
  typedef std::vector<std::pair<std::string, std::string> > HeaderVecType;

  // ヘッダー部における改行文字(LF or CRLF)
  std::string newline_chars_;

  ResponseType response_type_;

  std::map<std::string, std::string> headers_;

  utils::ByteVector body_;

 public:
  CgiResponse();
  CgiResponse(const CgiResponse &rhs);
  CgiResponse &operator=(const CgiResponse &rhs);
  ~CgiResponse();

  // buffer にはCGIの出力すべてが含まれている必要がある｡
  Result<void> Parse(utils::ByteVector &buffer);

  // ========================================================================
  // Getter and Setter
  ResponseType GetResponseType() const;
  const std::map<std::string, std::string> &GetHeaders();
  const utils::ByteVector &GetBody();

 private:
  // RFC3875(CGI/1.1) 2.2 Bacic Rules で定義されている BNF 規則
  static const std::string kLowalpha;
  static const std::string kHialpha;
  static const std::string kAlpha;
  static const std::string kDigit;
  static const std::string kSeparator;
  static const std::string kCtl;
  static const std::string kChar;
  static const std::string kCharExceptCtlAndSeparator;
  static const std::string kQdtext;

  // 改行文字を決定する
  Result<void> DetermineNewlineChars(utils::ByteVector &buffer);

  ResponseType IdentifyResponseType(utils::ByteVector &buffer);

  Result<void> SetHeadersFromBuffer(utils::ByteVector &buffer);

  Result<void> SetBodyFromBuffer(utils::ByteVector &buffer);

  // response-type
  // によってはヘッダーにデフォルト値が設定されていたりするので､設定する
  void AdjustHeadersBasedOnResponseType();

  // buffer からヘッダー部分を取り出す｡
  // pair->first がヘッダー名, pair->second が値になっている
  HeaderVecType GetHeaderVecFromBuffer(utils::ByteVector &buffer);

  // document-response = Content-Type [ Status ] *other-field NL response-body
  bool IsDocumentResponse(const HeaderVecType &headers, bool has_body);

  // local-redir-response = local-Location NL
  bool IsLocalRedirectResponse(const HeaderVecType &headers, bool has_body);

  // client-redir-response = client-Location *extension-field NL
  bool IsClientRedirectResponse(const HeaderVecType &headers, bool has_body);

  // client-redirdoc-response = client-Location Status Content-Type *other-field
  // NL response-body
  bool IsClientRedirectResponseWithDocument(const HeaderVecType &headers,
                                            bool has_body);

  // query-string や fragment を構成する uric をチェックする
  // uric         = reserved | unreserved | escaped
  bool IsComposedOfUriC(const std::string &str);

  // local-pathquery = abs-path [ "?" query-string ]
  bool IsLocalPathQuery(const std::string &pathquery);

  bool IsAbsoluteUri(const std::string &uri);

  // fragment-URI    = absoluteURI [ "#" fragment ]
  bool IsFragmentUri(const std::string &uri);

  // field-name      = token
  // token         = 1*<any CHAR except CTLs or separators>
  bool IsValidHeaderKey(const std::string &key);

  // field-content   = *( token | separator | quoted-string )
  // quoted-string = <"> *qdtext <">
  bool IsValidHeaderValue(const std::string &key);
};

}  // namespace cgi

#endif
