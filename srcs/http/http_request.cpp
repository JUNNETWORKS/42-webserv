#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

HttpRequest::HttpRequest() : phase_(kRequestLine) {
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
    phase_ = rhs.phase_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {}

void HttpRequest::AppendDataToBuffer(HttpRequest::Byte *buf, size_t size) {
  buffer_.insert(buffer_.end(), buf, buf + size);
  printf("current buf len: %lu\n", buffer_.size());
}

std::string HttpRequest::ExtractFromBuffer(const char *pos) {
  const char *buffer_ptr = reinterpret_cast<const char *>(buffer_.data());
  size_t size = pos - buffer_ptr;
  std::string res(buffer_ptr, size);
  buffer_.erase(buffer_.begin(), buffer_.begin() + size);

  return res;
};

const char *HttpRequest::FindCrlf() {
  buffer_.push_back('\0');
  const char *res = std::strstr(reinterpret_cast<const char *>(buffer_.data()),
                                kCrlf.c_str());
  buffer_.pop_back();
  return res;
};

bool HttpRequest::CompareBufferHead(const std::string &str) {
  if (buffer_.size() < str.size())
    return false;
  return std::strncmp(reinterpret_cast<const char *>(buffer_.data()),
                      str.c_str(), str.size()) == 0;
};

void HttpRequest::ParseRequest() {
  if (phase_ == kRequestLine)
    ParseRequestLine();
  if (phase_ == kHeaderField)
    ParseHeaderField();
};

void HttpRequest::EraseBufferHead(size_t size) {
  buffer_.erase(buffer_.begin(), buffer_.begin() + size);
}

void HttpRequest::ParseRequestLine() {
  while (CompareBufferHead(kCrlf)) {
    EraseBufferHead(kCrlf.size());  //空行を読み取ばす
  }

  const char *crlf_pos = FindCrlf();
  if (crlf_pos != NULL) {
    std::string line = ExtractFromBuffer(crlf_pos);  // request-lineの解釈
    printf("request-line: %s\n", line.c_str());
    phase_ = kHeaderField;
    return;
  }
}

void HttpRequest::ParseHeaderField() {
  while (1) {
    if (CompareBufferHead(kHeaderBoundary)) {
      //先頭が\r\n\r\nなので終了処理
      phase_ = kBody;
      return;
    }

    if (CompareBufferHead(kCrlf)) {
      // HeaderBoundary判定用に残しておいたcrlfを削除
      EraseBufferHead(kCrlf.size());
    }

    const char *crlf_pos = FindCrlf();
    if (crlf_pos == NULL) {
      return;  // crlfがbuffer内に存在しない
    } else {
      std::string line = ExtractFromBuffer(crlf_pos);  // headerfieldの解釈
      printf("headerfield: %s\n", line.c_str());
    }
  }
};
};  // namespace http
