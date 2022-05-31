#include "http/http_response.hpp"

namespace http {

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &rhs) {
  *this = rhs;
}

HttpResponse &HttpResponse::operator=(const HttpResponse &rhs) {
  if (this != &rhs) {
    status_ = rhs.status_;
    headers_ = rhs.headers_;
    body_ = rhs.body_;
    buffer_ = rhs.buffer_;
    buf_position_ = buffer_.data() + (rhs.buf_position_ - rhs.buffer_.data());
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

}  // namespace http
