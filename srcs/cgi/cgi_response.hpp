#ifndef CGI_CGI_RESPONSE_HPP_
#define CGI_CGI_RESPONSE_HPP_

#include <map>
#include <string>

class CgiResponse {
  enum ResponseType {
    kDocumentResponse,
    kLocalRedirect,
    kClientRedirect,
    kClientRedirectWithDocument,
    kParseError,
  };
};

#endif
