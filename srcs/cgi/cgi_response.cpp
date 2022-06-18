#include "cgi/cgi_response.hpp"

#include "utils/string.hpp"

namespace cgi {

CgiResponse::CgiResponse() {}

CgiResponse::CgiResponse(const CgiResponse &rhs) {
  *this = rhs;
}

CgiResponse &CgiResponse::operator=(const CgiResponse &rhs) {
  if (this != &rhs) {
    response_type_ = rhs.response_type_;
    headers_ = rhs.headers_;
    body_ = rhs.body_;
  }
  return *this;
}

CgiResponse::~CgiResponse() {}

void CgiResponse::Parse(utils::ByteVector &buffer) {
  (void)buffer;
}

// ========================================================================
// Getter and Setter

CgiResponse::ResponseType CgiResponse::GetResponseType() const {
  return response_type_;
}

const std::map<std::string, std::string> &CgiResponse::GetHeaders() {
  return headers_;
}

const utils::ByteVector &CgiResponse::GetBody() {
  return body_;
}

}  // namespace cgi
