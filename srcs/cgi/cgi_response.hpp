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
    kParsing,
    kDocumentResponse,
    kLocalRedirect,
    kClientRedirect,
    kClientRedirectWithDocument,
    kParseError,
  };

 private:
  ResponseType response_type_;
  std::map<std::string, std::string> headers_;

  int status_;
  std::string status_msg_;

  std::string location_;
  utils::ByteVector body_;

 public:
  CgiResponse();
  CgiResponse(const CgiResponse &rhs);
  CgiResponse &operator=(const CgiResponse &rhs);
  ~CgiResponse();

  void ParseResponse(utils::ByteVector &buffer);
  http::HttpResponse ToHttpResponse();

  bool IsParsed();

  // ========================================================================
  // Getter and Setter
  ResponseType GetResponseType();
};

}  // namespace cgi

#endif
