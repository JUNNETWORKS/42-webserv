#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

HttpRequest::HttpRequest() : phase_(kRequestLine), parse_status_(OK) {
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
    parse_status_ = rhs.parse_status_;
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
    if (ParseMethod(line) == OK && ParsePath(line) == OK) {
      printf("method_: %s\n", method_.c_str());
      printf("path_: %s\n", path_.c_str());
      phase_ = kHeaderField;
    }
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

bool HttpRequest::TryExtractBeforeWhiteSpace(std::string &src,
                                             std::string &dest) {
  size_t white_space_pos = src.find(" ");
  if (white_space_pos == std::string::npos) {
    return false;
  }
  dest = src.substr(0, white_space_pos);
  src.erase(0, white_space_pos + 1);
  return true;
}

HttpStatus HttpRequest::ParseMethod(std::string &str) {
  if (TryExtractBeforeWhiteSpace(str, method_) == false) {
    return parse_status_ = BAD_REQUEST;
  }

  if (method_ == method_strs::kGet || method_ == method_strs::kDelete ||
      method_ == method_strs::kPost) {
    // TODO 501 (Not Implemented)を判定する
    return parse_status_ = OK;
  }
  return parse_status_ = BAD_REQUEST;
}

HttpStatus HttpRequest::ParsePath(std::string &str) {
  if (TryExtractBeforeWhiteSpace(str, path_) == false) {
    return parse_status_ = BAD_REQUEST;
  }

  if (true) {  // TODO 長いURLの時414(URI Too Long)を判定する
    return parse_status_ = OK;
  }
  return parse_status_ = BAD_REQUEST;
}

};  // namespace http
