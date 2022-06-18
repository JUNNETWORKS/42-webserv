#ifndef CGI_CGI_RESPONSE_HPP_
#define CGI_CGI_RESPONSE_HPP_

#include <map>
#include <string>

#include "http/http_response.hpp"
#include "utils/ByteVector.hpp"

namespace cgi {

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
  ResponseType response_type_;

  std::map<std::string, std::string> headers_;

  utils::ByteVector body_;

 public:
  CgiResponse();
  CgiResponse(const CgiResponse &rhs);
  CgiResponse &operator=(const CgiResponse &rhs);
  ~CgiResponse();

  // buffer にはCGIの出力すべてが含まれている必要がある｡
  void Parse(utils::ByteVector &buffer);

  // ========================================================================
  // Getter and Setter
  ResponseType GetResponseType() const;
  const std::map<std::string, std::string> &GetHeaders();
  const utils::ByteVector &GetBody();
};

}  // namespace cgi

#endif
