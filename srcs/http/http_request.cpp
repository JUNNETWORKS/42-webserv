#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

HttpRequest::HttpRequest()
    : minor_version_(-1), phase_(kRequestLine), parse_status_(OK) {
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
    minor_version_ = rhs.minor_version_;
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
  if (phase_ == kBody)
    ParseBody();
  PrintRequestInfo();
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
    if (InterpretMethod(line) == OK && InterpretPath(line) == OK &&
        InterpretVersion(line) == OK) {
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
      InterpretHeaderField(line);
    }
  }
};

void HttpRequest::ParseBody() {
  // TODO Content-Lengthの判定,Bodyのパース
  phase_ = kParsed;
}

std::string HttpRequest::TrimWhiteSpace(std::string &str) {
  size_t start_pos = str.find_first_not_of(" ");
  size_t end_pos = str.find_last_not_of(" ");
  if (start_pos == std::string::npos)
    str.erase(str.begin(), str.end());
  str.erase(str.begin(), str.begin() + start_pos);
  str.erase(str.begin() + end_pos, str.end());
  return str;
}

HttpStatus HttpRequest::InterpretHeaderField(std::string &str) {
  size_t collon_pos = str.find_first_of(":");
  size_t white_space_pos = str.find_first_of(" ");

  if (collon_pos == std::string::npos || collon_pos == 0 ||
      (white_space_pos != std::string::npos && white_space_pos < collon_pos))
    return parse_status_ = BAD_REQUEST;

  std::string header = str.substr(0, collon_pos);
  std::transform(header.begin(), header.end(), header.begin(), toupper);
  str.erase(0, collon_pos + 1);
  std::string field = TrimWhiteSpace(str);

  headers_[header].push_back(field);
  phase_ = kBody;
  return parse_status_ = OK;
}

bool HttpRequest::TryExtractBeforeWhiteSpace(std::string &src,
                                             std::string &dest) {
  size_t white_space_pos = src.find_first_of(" ");
  if (white_space_pos == std::string::npos) {
    return false;
  }
  dest = src.substr(0, white_space_pos);
  src.erase(0, white_space_pos + 1);
  return true;
}

HttpStatus HttpRequest::InterpretMethod(std::string &str) {
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

HttpStatus HttpRequest::InterpretPath(std::string &str) {
  if (TryExtractBeforeWhiteSpace(str, path_) == false) {
    return parse_status_ = BAD_REQUEST;
  }

  if (true) {  // TODO 長いURLの時414(URI Too Long)を判定する
    return parse_status_ = OK;
  }
  return parse_status_ = BAD_REQUEST;
}

bool HttpRequest::IsCorrectHTTPVersion(const std::string &str) {
  if (str.find(kExpectMajorVersion) != 0)  // HTTP/ 1.0とかを弾く
    return false;
  std::string minor_ver = str.substr(kExpectMajorVersion.size(),
                                     str.size() - kExpectMajorVersion.size());

  if (minor_ver.size() == 0 ||
      minor_ver.size() >
          kMinorVersionDigitLimit)  // nginxだと 999はOK 1000はだめ
    return false;                   // "HTTP/1."を弾く

  for (std::string::iterator it = minor_ver.begin(); it != minor_ver.end();
       it++) {  // HTTP/1.hogeとかを弾く
    if (std::isdigit(*it) == false)
      return false;
  }
  return true;
}

HttpStatus HttpRequest::InterpretVersion(std::string &str) {
  size_t http_name_pos = str.find(kHttpVersionPrefix);
  if (http_name_pos != 0)
    return parse_status_ = BAD_REQUEST;

  str.erase(0, kHttpVersionPrefix.size());

  if (IsCorrectHTTPVersion(str)) {
    // HTTP1.~ が保証される
    str.erase(0, kExpectMajorVersion.size());
    minor_version_ = std::atoi(str.c_str());
    return parse_status_ = OK;
  } else if (std::isdigit(str[0]) == true && str[0] != '0' && str[0] != '1') {
    // HTTP2.0とかHTTP/2hogeとか
    return parse_status_ = HTTP_VERSION_NOT_SUPPORTED;
  } else {
    // 0.9とかHTTP/hogeとか
    return parse_status_ = BAD_REQUEST;
  }
}

void HttpRequest::PrintRequestInfo() {
  printf("====Request Parser====\n");
  printf("ParseStatus_: %d\n", parse_status_);
  printf("Phase: %d\n", phase_);
  if (parse_status_ == OK) {
    printf("method_: %s\n", method_.c_str());
    printf("path_: %s\n", path_.c_str());
    printf("version_: %d\n", minor_version_);
    for (std::map<std::string, std::vector<std::string> >::iterator it =
             headers_.begin();
         it != headers_.end(); it++) {
      printf("%s: ", (*it).first.c_str());
      for (std::vector<std::string>::iterator sit = (*it).second.begin();
           sit != (*it).second.end(); sit++) {
        printf("%s, ", (*sit).c_str());
      }
      printf("\n");
    }
  }
  printf("=====================\n");
};

bool HttpRequest::IsCorrectRequest() {
  return phase_ == kParsed && parse_status_ == OK;
};

};  // namespace http
