#include "cgi/cgi_response.hpp"

namespace cgi {

CgiResponse::CgiResponse() {}

CgiResponse::CgiResponse(const CgiResponse &rhs) {}

CgiResponse &CgiResponse::operator=(const CgiResponse &rhs) {}

CgiResponse::~CgiResponse() {}

void CgiResponse::Parse(utils::ByteVector &buffer) {}

http::HttpResponse CgiResponse::ToHttpResponse() {}

bool CgiResponse::IsParsed() {}

// ========================================================================
// Getter and Setter
CgiResponse::ResponseType CgiResponse::GetResponseType() {}

}  // namespace cgi
