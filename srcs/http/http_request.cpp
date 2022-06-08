#include "http/http_request.hpp"

#include <stdio.h>

namespace http {

namespace {

bool IsTcharString(const std::string &str);
bool IsCorrectHTTPVersion(const std::string &str);
bool TryCutSubstrBeforeWhiteSpace(std::string &src, std::string &dest);
bool ParseHeaderFieldValue(std::string &str, std::vector<std::string> &vec);
}  // namespace

HttpRequest::HttpRequest()
    : method_(""),
      path_(""),
      minor_version_(-1),
      headers_(),
      phase_(kRequestLine),
      parse_status_(OK),
      body_(),
      body_size_(0) {}

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
    body_size_ = rhs.body_size_;
  }
  return *this;
}

HttpRequest::~HttpRequest() {}

//========================================================================
// getter

const std::string &HttpRequest::GetPath() const {
  return path_;
}

//========================================================================
// Parse系関数　内部でInterpret系関数を呼び出す　主にphaseで動作管理

void HttpRequest::ParseRequest(utils::ByteVector &buffer) {
  // TODO 長すぎるbufferは捨ててエラーにする
  if (phase_ == kRequestLine)
    phase_ = ParseRequestLine(buffer);
  if (phase_ == kHeaderField)
    phase_ = ParseHeaderField(buffer);
  if (phase_ == kBodySize)
    phase_ = ParseBodySize();
  if (phase_ == kBody)
    phase_ = ParseBody(buffer);
  PrintRequestInfo();
}

HttpRequest::ParsingPhase HttpRequest::ParseRequestLine(
    utils::ByteVector &buffer) {
  while (buffer.CompareHead(kCrlf)) {
    buffer.EraseHead(kCrlf.size());
  }

  utils::ByteVector::iterator it = buffer.FindString(kCrlf);
  if (it != buffer.end()) {
    std::string line = buffer.CutSubstrBeforePos(it);
    if (InterpretMethod(line) == OK && InterpretPath(line) == OK &&
        InterpretVersion(line) == OK) {
      return kHeaderField;
    } else {
      return kError;
    }
  }
  return kRequestLine;
}

HttpRequest::ParsingPhase HttpRequest::ParseHeaderField(
    utils::ByteVector &buffer) {
  if (buffer.FindString(kHeaderBoundary) == buffer.end()) {
    return kHeaderField;
  }

  while (1) {
    if (buffer.CompareHead(kHeaderBoundary)) {
      buffer.EraseHead(kHeaderBoundary.size());
      //先頭が\r\n\r\nなので終了処理
      return kBodySize;
    }

    if (buffer.CompareHead(kCrlf)) {
      // HeaderBoundary判定用に残しておいたcrlfを削除
      buffer.EraseHead(kCrlf.size());
    }

    utils::ByteVector::iterator it = buffer.FindString(kCrlf);
    if (it == buffer.end()) {
      return kHeaderField;  // crlfがbuffer内に存在しない
    } else {
      std::string line = buffer.CutSubstrBeforePos(it);  // headerfieldの解釈
      if (InterpretHeaderField(line) != OK)
        return kError;
    }
  }
}

HttpRequest::ParsingPhase HttpRequest::ParseBodySize() {
  if (DecideBodySize() != OK)
    return kError;
  return kBody;
}

HttpRequest::ParsingPhase HttpRequest::ParseBody(utils::ByteVector &buffer) {
  if (body_size_ == 0)
    return kParsed;

  size_t request_size = body_size_ - body_.size();
  if (buffer.size() <= request_size) {
    body_.insert(body_.end(), buffer.begin(), buffer.end());
    buffer.clear();
  } else {
    body_.insert(body_.end(), buffer.begin(), buffer.begin() + request_size);
    buffer.erase(buffer.begin(), buffer.begin() + request_size);
  }
  return body_.size() == body_size_ ? kParsed : kBody;
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

  if (collon_pos == std::string::npos || collon_pos == 0 ||
      IsTcharString(str.substr(0, collon_pos)) == false)
    return parse_status_ = BAD_REQUEST;

  std::string header = str.substr(0, collon_pos);
  std::transform(header.begin(), header.end(), header.begin(), toupper);
  str.erase(0, collon_pos + 1);
  std::string field = utils::TrimString(str, kOWS);

  if (ParseHeaderFieldValue(str, headers_[header]) == false) {
    return parse_status_ = BAD_REQUEST;
  }
  return parse_status_ = OK;
}

HttpStatus HttpRequest::InterpretContentLength(
    const HeaderMap::mapped_type &length_header) {
  // ヘッダ値が相違する，複数の Content-Length ヘッダが在る†
  //妥当でない値をとる Content - Length ヘッダが在る
  if (length_header.size() != 1)
    return parse_status_ = BAD_REQUEST;

  if (utils::Stoul(body_size_, length_header.front()) == false)
    return parse_status_ = BAD_REQUEST;

  const unsigned long kMaxSize = 1073741824;  // TODO config読み込みに変更
  if (body_size_ > kMaxSize)
    return parse_status_ = PAYLOAD_TOO_LARGE;

  return parse_status_ = OK;
}

//========================================================================
// Is系関数　外部から状態取得
bool HttpRequest::IsParsed() {
  return phase_ == kParsed;
}
bool HttpRequest::IsCorrectStatus() {
  return parse_status_ == OK;
}

// ========================================================================
// Getter and Setter
const std::vector<std::string> &HttpRequest::GetHeader(
    const std::string header) {
  return headers_[header];
}

//========================================================================
// Helper関数

HttpStatus HttpRequest::DecideBodySize() {
  // https://triple-underscore.github.io/RFC7230-ja.html#message.body.length

  body_size_ = 0;

  HeaderMap::iterator encoding_header_it = headers_.find("TRANSFER-ENCODING");
  HeaderMap::iterator length_header_it = headers_.find("CONTENT-LENGTH");
  bool has_encoding_header = encoding_header_it != headers_.end();
  bool has_length_header = length_header_it != headers_.end();

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

  if (has_length_header)
    return InterpretContentLength((*length_header_it).second);

  return OK;
}

namespace {

// tcharのみの文字列か判定
// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*"
//         "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"/ DIGIT / ALPHA
// ヘッダ名が入ってきてコロンは含まない想定
bool IsTcharString(const std::string &str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (std::isalnum(str[i]) == false &&
        kNotAlnumTchars.find(str[i]) == std::string::npos) {
      return false;
    }
  }
  return true;
}

// RFC7230から読み解ける仕様をできるかぎり実装
// DQUOTEで囲まれている文字列の内部でのみエスケープが効く
//　エスケープされてないDQUOTEは取り除く
// DQUOTEで囲まれていないカンマまでをreturnする。カンマはstrに残る
// ペアの存在しないDQUOTEは文字扱いでのこる。
bool CutSubstrHeaderValue(std::string &res, std::string &str) {
  bool is_quoting = false;
  std::string result;
  std::string::iterator it = str.begin();
  std::string::iterator quote_pos;
  for (; it != str.end(); it++) {
    if (is_quoting) {
      if (*it == '"') {
        is_quoting = false;
      } else if (*it == '\\' && it + 1 != str.end()) {
        it++;
        result += *it;
      } else {
        result += *it;
      }
    } else {
      if (*it == '"') {
        quote_pos = it;
        is_quoting = true;
      } else if (*it == ',') {
        break;
      } else {
        result += *it;
      }
    }
  }
  if (is_quoting) {
    return false;
  }
  res = result;
  str.erase(str.begin(), it);
  return true;
}

// strにヘッダの:以降を受け取り、splitしてvecにつめる
//  e.g. str = If-Match: "strong", W/"weak", "oops, a \"comma\""
//  returnは 'strong', 'W/weak'  'oops, a "comma"'
bool ParseHeaderFieldValue(std::string &str, std::vector<std::string> &vec) {
  std::string value;

  while (!str.empty()) {
    utils::TrimString(str, kOWS);
    if (CutSubstrHeaderValue(value, str) == false) {
      return false;
    }
    vec.push_back(value);
    if (!str.empty() && str[0] == ',') {
      str.erase(str.begin());
    }
  }
  return true;
}

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
    printf("body_size: %ld\n", body_size_);
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

    printf("body:\n");
    body_.push_back('\0');
    printf("%s\n", reinterpret_cast<const char *>(body_.data()));
    body_.pop_back();
  }
  printf("=====================\n");
}

bool HttpRequest::IsCorrectRequest() {
  return phase_ == kParsed && parse_status_ == OK;
}

}  // namespace http
