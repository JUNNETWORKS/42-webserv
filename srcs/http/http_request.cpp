#include "http/http_request.hpp"

#include <stdio.h>

#include "result/result.hpp"

namespace http {

namespace {

using namespace result;
bool IsMethod(const std::string &token);
bool IsObsFold(const utils::ByteVector &buf);
bool IsTcharString(const std::string &str);
bool IsCorrectHTTPVersion(const std::string &str);
Result<std::vector<std::string> > ParseHeaderFieldValue(std::string &str);
std::pair<Chunk::ChunkStatus, Chunk> CheckChunkReceived(
    utils::ByteVector &buffer);
}  // namespace

HttpRequest::HttpRequest(const config::Config &config)
    : config_(config),
      method_(""),
      path_(""),
      minor_version_(-1),
      headers_(),
      phase_(kRequestLine),
      parse_status_(OK),
      body_(),
      body_size_(0),
      is_chunked_(false),
      has_obs_fold_(false) {}

HttpRequest::HttpRequest(const HttpRequest &rhs)
    : config_(rhs.config_),
      method_(rhs.method_),
      path_(rhs.path_),
      minor_version_(rhs.minor_version_),
      headers_(rhs.headers_),
      phase_(rhs.phase_),
      parse_status_(rhs.parse_status_),
      body_(rhs.body_),
      body_size_(rhs.body_size_),
      is_chunked_(rhs.is_chunked_) {}

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

  Result<size_t> pos = buffer.FindString(kCrlf);
  if (pos.IsErr())
    return kRequestLine;

  const std::string str = buffer.SubstrBeforePos(pos.Ok());
  if (std::count(str.begin(), str.end(), ' ') != 2) {
    parse_status_ = BAD_REQUEST;
    return kError;
  }

  std::vector<std::string> tokens = utils::SplitString(str, " ");
  if (InterpretMethod(tokens[0]) == OK && InterpretPath(tokens[1]) == OK &&
      InterpretVersion(tokens[2]) == OK) {
    buffer.erase(buffer.begin(), buffer.begin() + pos.Ok());
    return kHeaderField;
  }
  return kError;
}

HttpRequest::ParsingPhase HttpRequest::ParseHeaderField(
    utils::ByteVector &buffer) {
  Result<size_t> boundary_pos = buffer.FindString(kHeaderBoundary);
  if (boundary_pos.IsErr())
    return kHeaderField;

  while (1) {
    if (IsObsFold(buffer)) {  //先頭がobs-foldの時
      parse_status_ = BAD_REQUEST;
      return kError;
    } else if (buffer.CompareHead(kHeaderBoundary)) {
      //先頭が\r\n\r\nなので終了処理
      buffer.EraseHead(kHeaderBoundary.size());
      return kBodySize;
    } else {
      buffer.EraseHead(kCrlf.size());
    }

    Result<size_t> crlf_pos = buffer.FindString(kCrlf);
    std::string field_buffer_str = buffer.CutSubstrBeforePos(crlf_pos.Ok());
    if (InterpretHeaderField(field_buffer_str) != OK)
      return kError;
  }
}

HttpRequest::ParsingPhase HttpRequest::ParseBodySize() {
  if (DecideBodySize() != OK)
    return kError;
  return kBody;
}

HttpRequest::ParsingPhase HttpRequest::ParseBody(utils::ByteVector &buffer) {
  if (is_chunked_) {
    return ParseChunkedBody(buffer);
  } else {
    return ParsePlainBody(buffer);
  }
}

HttpRequest::ParsingPhase HttpRequest::ParsePlainBody(
    utils::ByteVector &buffer) {
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

HttpRequest::ParsingPhase HttpRequest::ParseChunkedBody(
    utils::ByteVector &buffer) {
  while (buffer.empty() == false) {
    std::pair<Chunk::ChunkStatus, Chunk> chunk_pair =
        CheckChunkReceived(buffer);
    switch (chunk_pair.first) {
      case Chunk::kWaiting:  // bufferにchunkが届ききっていない。
        return phase_;
      case Chunk::kErrorBadRequest:
        parse_status_ = BAD_REQUEST;
        return kError;
      case Chunk::kErrorLength:
        parse_status_ = PAYLOAD_TOO_LARGE;
        return kError;
      default:
        break;
    }

    Chunk chunk = chunk_pair.second;
    buffer.erase(buffer.begin(),
                 buffer.begin() + chunk.size_str.size() + kCrlf.size());
    if (chunk_pair.second.data_size == 0)
      return kParsed;
    body_.insert(body_.end(), buffer.begin(), buffer.begin() + chunk.data_size);
    buffer.erase(buffer.begin(),
                 buffer.begin() + chunk.data_size + kCrlf.size());
  }
  return phase_;
}

//========================================================================
// Interpret系関数　文字列を解釈する関数　主にparse_statusで動作管理(OKじゃなくなったら次は実行されない)

HttpStatus HttpRequest::InterpretMethod(const std::string &token) {
  if (IsMethod(token)) {
    method_ = token;
    return parse_status_ = OK;
  } else if (token.empty() || IsTcharString(token) == false) {
    return parse_status_ = BAD_REQUEST;
  } else {
    return parse_status_ = NOT_IMPLEMENTED;
  }
}

HttpStatus HttpRequest::InterpretPath(const std::string &token) {
  if (token.size() > kMaxUriLength) {
    return parse_status_ = URI_TOO_LONG;
  } else {
    path_ = token;
    return parse_status_ = OK;
  }
}

HttpStatus HttpRequest::InterpretVersion(const std::string &token) {
  if (utils::ForwardMatch(token, kHttpVersionPrefix)) {
    const std::string str = token.substr(kHttpVersionPrefix.size());
    if (IsCorrectHTTPVersion(str)) {
      // HTTP1.~ が保証される
      minor_version_ =
          std::atoi(str.substr(kExpectMajorVersion.size()).c_str());
      return parse_status_ = OK;
    } else if (str.empty() == false && '2' <= str[0] && str[0] <= '9') {
      // HTTP2.0とかHTTP/2hogeとか
      return parse_status_ = HTTP_VERSION_NOT_SUPPORTED;
    }
  }
  // 0.9とかHTTP/hogeとかHTTP/ 1.1とかHTTP/1. 1とか
  return parse_status_ = BAD_REQUEST;
}

HttpStatus HttpRequest::InterpretHeaderField(const std::string &str) {
  size_t collon_pos = str.find_first_of(":");

  if (collon_pos == std::string::npos || collon_pos == 0)
    return parse_status_ = BAD_REQUEST;

  std::string header = str.substr(0, collon_pos);
  std::string value_str = str.substr(collon_pos + 1);
  Result<std::vector<std::string> > result = ParseHeaderFieldValue(value_str);
  if (result.IsErr() || IsTcharString(header) == false) {
    return parse_status_ = BAD_REQUEST;
  }
  std::transform(header.begin(), header.end(), header.begin(), toupper);
  headers_[header] = result.Ok();
  return parse_status_ = OK;
}

HttpStatus HttpRequest::InterpretContentLength(
    const HeaderMap::mapped_type &length_header) {
  // ヘッダ値が相違する，複数の Content-Length ヘッダが在る†
  //妥当でない値をとる Content - Length ヘッダが在る
  if (length_header.size() != 1)
    return parse_status_ = BAD_REQUEST;

  Result<unsigned long> result = utils::Stoul(length_header.front());
  if (result.IsErr())
    return parse_status_ = BAD_REQUEST;

  body_size_ = result.Ok();
  const unsigned long kMaxSize = 1073741824;  // TODO config読み込みに変更
  if (body_size_ > kMaxSize)
    return parse_status_ = PAYLOAD_TOO_LARGE;

  return parse_status_ = OK;
}

HttpStatus HttpRequest::InterpretTransferEncoding(
    const HeaderMap::mapped_type &encoding_header) {
  if (encoding_header.size() == 1 && encoding_header.front() == "chunked") {
    // 最終転送符号法はチャンク化である
    is_chunked_ = true;
    return parse_status_ = OK;
  } else {
    // 最終転送符号法はチャンク化でない
    // webservではchunkedのみ許容
    return parse_status_ = NOT_IMPLEMENTED;
  }
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
const std::vector<std::string> &HttpRequest::GetHeader(std::string header) {
  std::transform(header.begin(), header.end(), header.begin(), toupper);
  return headers_[header];
}

HttpStatus HttpRequest::GetParseStatus() const {
  return parse_status_;
}

const utils::ByteVector &HttpRequest::GetBody() {
  return body_;
}

//========================================================================
// Helper関数

HttpStatus HttpRequest::DecideBodySize() {
  // https://triple-underscore.github.io/RFC7230-ja.html#message.body.length

  HeaderMap::iterator encoding_header_it = headers_.find("TRANSFER-ENCODING");
  HeaderMap::iterator length_header_it = headers_.find("CONTENT-LENGTH");
  bool has_encoding_header = encoding_header_it != headers_.end();
  bool has_length_header = length_header_it != headers_.end();

  if (has_encoding_header && has_length_header) {
    return parse_status_ = BAD_REQUEST;
  }

  if (has_encoding_header)
    return InterpretTransferEncoding((*encoding_header_it).second);

  if (has_length_header)
    return InterpretContentLength((*length_header_it).second);

  return OK;
}

namespace {

bool IsMethod(const std::string &token) {
  return token == method_strs::kGet || token == method_strs::kDelete ||
         token == method_strs::kPost;
}

bool IsObsFold(const utils::ByteVector &buf) {
  return buf.CompareHead(kCrlf + " ") || buf.CompareHead(kCrlf + "\t");
}

// Chunk内にCRLFがあること、CRLFがチャンクの末尾についている事を検証する。
bool ValidateChunkDataFormat(const Chunk &chunk, utils::ByteVector &buffer) {
  utils::ByteVector chunk_data_bytes = utils::ByteVector(
      buffer.begin() + chunk.size_str.size() + kCrlf.size(), buffer.end());
  Result<size_t> res = chunk_data_bytes.FindString(kCrlf);
  if (res.IsOk()) {
    return res.Ok() == chunk.data_size;
  }
  return false;
}

std::pair<Chunk::ChunkStatus, Chunk> CheckChunkReceived(
    utils::ByteVector &buffer) {
  Chunk res;

  Result<size_t> pos = buffer.FindString(kCrlf);
  if (pos.IsErr())
    return std::make_pair(Chunk::kWaiting, res);
  res.size_str = buffer.SubstrBeforePos(pos.Ok());

  Result<unsigned long> convert_res =
      utils::Stoul(res.size_str, utils::kHexadecimal);
  if (convert_res.IsErr())
    return std::make_pair(Chunk::kErrorBadRequest, res);
  res.data_size = convert_res.Ok();

  const unsigned long kMaxSize = 1073741824;  // TODO config読み込みに変更
  if (res.data_size >= kMaxSize) {
    return std::make_pair(Chunk::kErrorLength, res);
  }

  if (res.data_size == 0) {
    return std::make_pair(Chunk::kReceived, res);
  }

  unsigned long expect_buffer_size =
      res.size_str.size() + kCrlf.size() + res.data_size + kCrlf.size();
  if (buffer.size() < expect_buffer_size)
    return std::make_pair(Chunk::kWaiting, res);

  if (ValidateChunkDataFormat(res, buffer) == false)
    return std::make_pair(Chunk::kErrorBadRequest, res);

  return std::make_pair(Chunk::kReceived, res);
}

bool IsTchar(const char c) {
  return std::isalnum(c) || kTcharsWithoutAlnum.find(c) != std::string::npos;
}

// tcharのみの文字列か判定
// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*"
//         "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"/ DIGIT / ALPHA
// ヘッダ名が入ってきてコロンは含まない想定
bool IsTcharString(const std::string &str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (IsTchar(str[i]) == false) {
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
Result<std::string> CutSubstrHeaderValue(std::string &str) {
  bool is_quoting = false;
  std::string result;
  std::string::iterator it = str.begin();
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
        is_quoting = true;
      } else if (*it == ',') {
        break;
      } else {
        result += *it;
      }
    }
  }
  if (is_quoting) {
    return Error();
  }
  str.erase(str.begin(), it);
  return result;
}

// strにヘッダの:以降を受け取り、splitしてvecにつめる
//  e.g. str = If-Match: "strong", W/"weak", "oops, a \"comma\""
//  returnは 'strong', 'W/weak'  'oops, a "comma"'
Result<std::vector<std::string> > ParseHeaderFieldValue(std::string &str) {
  std::vector<std::string> vec;

  while (!str.empty()) {
    utils::TrimString(str, kOWS);
    Result<std::string> result = CutSubstrHeaderValue(str);
    if (result.IsErr()) {
      return result.Err();
    }
    vec.push_back(result.Ok());
    if (!str.empty() && str[0] == ',') {
      str.erase(str.begin());
    }
  }
  return vec;
}

bool IsCorrectHTTPVersion(const std::string &str) {
  return str.size() > kExpectMajorVersion.size() &&
         str.size() <= kExpectMajorVersion.size() + kMinorVersionDigitLimit &&
         utils::ForwardMatch(str, kExpectMajorVersion) &&
         utils::IsDigits(str.substr(kExpectMajorVersion.size()));
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
    printf("is_chuked_: %d\n", is_chunked_);
    printf("has_obs_fold_: %d\n", has_obs_fold_);
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
