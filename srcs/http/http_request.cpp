#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

HttpRequest::HttpRequest() {
  this->buffer_.reserve(reserve_size_);
}

HttpRequest::HttpRequest(const HttpRequest &rhs) {
  *this = rhs;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &rhs) {
  if (this != &rhs) {
    method_ = rhs.method_;
    path_ = rhs.path_;
    headers_ = rhs.headers_;
    body_ = rhs.body_;
    buffer_ = rhs.buffer_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {}

void HttpRequest::AppendDataToBuffer(HttpRequest::Byte *buf, size_t size) {
  buffer_.insert(buffer_.end(), buf, buf + size);
  printf("current buf len: %lu\n", buffer_.size());
}

// TODO
void HttpRequest::ParseOneLine() {}

};  // namespace http
