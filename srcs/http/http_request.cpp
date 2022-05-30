#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

namespace {
bool IsCorrectHTTPVersion(const std::string &str);
bool TryCutSubstrBeforeWhiteSpace(std::string &src, std::string &dest);
}  // namespace

HttpRequest::HttpRequest()
    : method_(""),
      path_(""),
      minor_version_(-1),
      headers_(),
      phase_(kRequestLine),
      parse_status_(OK),
      body_(),
      buffer_() {}

HttpRequest::HttpRequest(const HttpRequest &rhs) {
  *this = rhs;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &rhs) {
  if (this != &rhs) {
    method_ = rhs.method_;
    path_ = rhs.path_;
    minor_version_ = rhs.minor_version_;
    headers_ = rhs.headers_;
    phase_ = rhs.phase_;
    parse_status_ = rhs.parse_status_;
    body_ = rhs.body_;
    buffer_ = rhs.buffer_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {}

//========================================================================
// Parse系関数　内部でInterpret系関数を呼び出す　主にphaseで動作管理

void HttpRequest::ParseRequest() {
  if (phase_ == kRequestLine)
    phase_ = ParseRequestLine();
  if (phase_ == kHeaderField)
    phase_ = ParseHeaderField();
  if (phase_ == kBody)
    phase_ = ParseBody();
  PrintRequestInfo();
};

HttpRequest::ParsingPhase HttpRequest::ParseRequestLine() {
  while (buffer_.CompareHead(kCrlf)) {
    buffer_.EraseHead(kCrlf.size());
  }

  utils::ByteVector::iterator it = buffer_.FindString(kCrlf);
  if (it != buffer_.end()) {
    std::string line = buffer_.CutSubstrBeforePos(it);
    if (InterpretMethod(line) == OK && InterpretPath(line) == OK &&
        InterpretVersion(line) == OK) {
      return kHeaderField;
    }
  }
  return phase_;
}

HttpRequest::ParsingPhase HttpRequest::ParseHeaderField() {
  while (1) {
    if (buffer_.CompareHead(kHeaderBoundary)) {
      //先頭が\r\n\r\nなので終了処理
      return kBody;
    }

    if (buffer_.CompareHead(kCrlf)) {
      // HeaderBoundary判定用に残しておいたcrlfを削除
      buffer_.EraseHead(kCrlf.size());
    }

    utils::ByteVector::iterator it = buffer_.FindString(kCrlf);
    if (it == buffer_.end()) {
      return phase_;  // crlfがbuffer内に存在しない
    } else {
      std::string line = buffer_.CutSubstrBeforePos(it);  // headerfieldの解釈
      InterpretHeaderField(line);
    }
  }
};

HttpRequest::ParsingPhase HttpRequest::ParseBody() {
  // TODO Content-Lengthの判定,Bodyのパース
  return kParsed;
}

//========================================================================
// Interpret系関数　文字列を解釈する関数　主にparse_statusで動作管理(OKじゃなくなったら次は実行されない)

HttpStatus HttpRequest::InterpretMethod(std::string &str) {
  if (TryCutSubstrBeforeWhiteSpace(str, method_) == false) {
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
  if (TryCutSubstrBeforeWhiteSpace(str, path_) == false) {
    return parse_status_ = BAD_REQUEST;
  }

  if (true) {  // TODO 長いURLの時414(URI Too Long)を判定する
    return parse_status_ = OK;
  }
  return parse_status_ = BAD_REQUEST;
}

HttpStatus HttpRequest::InterpretVersion(std::string &str) {
  if (!utils::ForwardMatch(str, kHttpVersionPrefix))
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

HttpStatus HttpRequest::InterpretHeaderField(std::string &str) {
  size_t collon_pos = str.find_first_of(":");
  size_t white_space_pos = str.find_first_of(" ");

  if (collon_pos == std::string::npos || collon_pos == 0 ||
      (white_space_pos != std::string::npos && white_space_pos < collon_pos))
    return parse_status_ = BAD_REQUEST;

  std::string header = str.substr(0, collon_pos);
  std::transform(header.begin(), header.end(), header.begin(), toupper);
  str.erase(0, collon_pos + 1);
  std::string field = utils::TrimWhiteSpace(str);

  // TODO ,区切りのsplit
  headers_[header].push_back(field);
  phase_ = kBody;
  return parse_status_ = OK;
}

//========================================================================
// Helper関数

HttpStatus HttpRequest::DecideBodySize(size_t &body_size) {
  // https://triple-underscore.github.io/RFC7230-ja.html#message.body.length

  body_size_ = 0;

  bool has_encoding_header =
      headers_.find("TRANSFER-ENCODING") != headers_.end();
  bool has_length_header = headers_.find("CONTENT-LENGTH") != headers_.end();

  if (has_encoding_header && has_length_header) {
    // TODO ステータスの検証　BAD_Requestは仮
    return parse_status_ = BAD_REQUEST;
  }

  if (has_encoding_header) {
    // TODO
    // 最終転送符号法はチャンク化である
    // 最終転送符号法はチャンク化でない
    return parse_status_ = OK;
  }

  if (has_length_header) {
    // ヘッダ値が相違する，複数の Content-Length ヘッダが在る†
    //妥当でない値をとる Content - Length ヘッダが在る
    return parse_status_ = OK;
  }

  return OK;
}

namespace {
bool IsCorrectHTTPVersion(const std::string &str) {
  if (!utils::ForwardMatch(str, kExpectMajorVersion))  // HTTP/ 1.0とかを弾く
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

bool TryCutSubstrBeforeWhiteSpace(std::string &buffer, std::string &res) {
  size_t white_space_pos = buffer.find_first_of(" ");
  if (white_space_pos == std::string::npos) {
    return false;
  }
  res = buffer.substr(0, white_space_pos);
  buffer.erase(0, white_space_pos + 1);
  return true;
}

}  // namespace

//========================================================================
// こいつらは多分そのうち消える

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
