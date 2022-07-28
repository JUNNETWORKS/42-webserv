#include "cgi/cgi_response.hpp"

#include <algorithm>

#include "http/http_status.hpp"
#include "utils/string.hpp"

namespace cgi {

const std::string CgiResponse::kLowalpha = "abcdefghijklmnopqrstuvwxyz";
const std::string CgiResponse::kHialpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string CgiResponse::kAlpha = kLowalpha + kHialpha;
const std::string CgiResponse::kDigit = "0123456789";
const std::string CgiResponse::kSeparator = "()<>@,;:\\\"/[]?={} \t";
const std::string CgiResponse::kCtl =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11"
    "\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f";
const std::string CgiResponse::kChar =
    kAlpha + kDigit + kSeparator + "!#$%&'*+-.`^_{|}~" + kCtl;
const std::string CgiResponse::kCharExceptCtlAndSeparator =
    kAlpha + kDigit + "!#$%&'*+-.`^_{|}~";
const std::string CgiResponse::kQdtext =
    kAlpha + kDigit + "()<>@,;:\\/[]?={} \t" + "!#$%&'*+-.`^_{|}~" + "\r\n";

namespace {

// generic-field   = field-name ":" [ field-value ] NL
// field-name が空文字列､もしくは ':' がない場合はエラー
Result<std::string> GetKeyOfHeader(const std::string &header);
Result<std::string> GetValueOfHeader(const std::string &header);

}  // namespace

CgiResponse::CgiResponse()
    : newline_chars_(""), response_type_(kNotIdentified) {}

CgiResponse::CgiResponse(const CgiResponse &rhs) {
  *this = rhs;
}

CgiResponse &CgiResponse::operator=(const CgiResponse &rhs) {
  if (this != &rhs) {
    newline_chars_ = rhs.newline_chars_;
    response_type_ = rhs.response_type_;
    headers_ = rhs.headers_;
    body_ = rhs.body_;
  }
  return *this;
}

CgiResponse::~CgiResponse() {}

CgiResponse::ResponseType CgiResponse::Parse(utils::ByteVector &buffer) {
  if (response_type_ == kNotIdentified) {
    // まだ response_type が決まっていない場合
    if (newline_chars_.empty() && DetermineNewlineChars(buffer).IsErr()) {
      // ステータス部とヘッダー部の上限サイズを超えている場合はエラー
      if (buffer.size() > kMaxStatusHeaderSize) {
        return response_type_ = kParseError;
      }
      // ヘッダーとボディの区切りが見つからない場合は kNotIdentified
      return response_type_ = kNotIdentified;
    }

    if (SetHeadersFromBuffer(buffer).IsErr()) {
      return response_type_ = kParseError;
    }

    response_type_ = IdentifyResponseType();
    if (response_type_ == kParseError) {
      return response_type_;
    }

    AdjustHeadersBasedOnResponseType();
  }
  if (response_type_ != kNotIdentified && response_type_ != kParseError) {
    if (buffer.size() && (response_type_ == kLocalRedirect ||
                          response_type_ == kClientRedirect)) {
      // response-body を保持することが許可されていないレスポンスタイプ
      return response_type_ = kParseError;
    }
    AppendBodyFromBuffer(ConvertToChunkResponse(buffer));
    buffer.clear();
  }

  return response_type_;
}

// ========================================================================
// Getter and Setter

CgiResponse::ResponseType CgiResponse::GetResponseType() const {
  return response_type_;
}

const CgiResponse::HeaderVecType &CgiResponse::GetHeaders() {
  return headers_;
}

Result<std::string> CgiResponse::GetHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  for (HeaderVecType::const_iterator it = headers_.begin();
       it != headers_.end(); ++it) {
    if (it->first == key) {
      return it->second;
    }
  }
  return Error();
}

utils::ByteVector &CgiResponse::GetBody() {
  return body_;
}

Result<void> CgiResponse::DetermineNewlineChars(
    const utils::ByteVector &buffer) {
  Result<size_t> lflf_res = buffer.FindString(kLF + kLF);
  Result<size_t> crlfcrlf_res = buffer.FindString(kCRLF + kCRLF);

  if (lflf_res.IsOk() && crlfcrlf_res.IsOk()) {
    size_t lflf_idx = lflf_res.Ok();
    size_t crlfcrlf_idx = crlfcrlf_res.Ok();
    if (lflf_idx < crlfcrlf_idx) {
      newline_chars_ = kLF;
    } else {
      newline_chars_ = kCRLF;
    }
  } else if (lflf_res.IsOk()) {
    newline_chars_ = kLF;
  } else if (crlfcrlf_res.IsOk()) {
    newline_chars_ = kCRLF;
  } else {
    return Error();
  }
  return Result<void>();
}

CgiResponse::ResponseType CgiResponse::IdentifyResponseType() const {
  assert(newline_chars_.size() > 0);

  if (IsDocumentResponse()) {
    return CgiResponse::kDocumentResponse;
  } else if (IsLocalRedirectResponse()) {
    return CgiResponse::kLocalRedirect;
  } else if (IsClientRedirectResponseWithDocument()) {
    return CgiResponse::kClientRedirectWithDocument;
  } else if (IsClientRedirectResponse()) {
    return CgiResponse::kClientRedirect;
  } else {
    return CgiResponse::kParseError;
  }
}

Result<void> CgiResponse::SetHeadersFromBuffer(utils::ByteVector &buffer) {
  Result<HeaderVecType> header_vec_res = GetHeaderVecFromBuffer(buffer);
  if (header_vec_res.IsErr()) {
    return Error();
  }
  headers_ = header_vec_res.Ok();

  // ヘッダー部を削除
  Result<size_t> headers_boundary_res =
      buffer.FindString(newline_chars_ + newline_chars_);
  if (headers_boundary_res.IsErr()) {
    return Error();
  }
  size_t headers_boundary = headers_boundary_res.Ok();
  buffer.EraseHead(headers_boundary + (newline_chars_ + newline_chars_).size());

  return Result<void>();
}

utils::ByteVector CgiResponse::ConvertToChunkResponse(utils::ByteVector data) {
  std::stringstream ss;
  while (data.empty() == false) {
    size_t chunk_size =
        data.size() < http::kMaxUriLength ? data.size() : http::kMaxUriLength;
    ss << std::hex << chunk_size;
    ss << http::kCrlf;
    ss << data.SubstrBeforePos(chunk_size);
    ss << http::kCrlf;
    data.erase(data.begin(), data.begin() + chunk_size);
  }
  return ss.str();
}

void CgiResponse::AppendLastChunk() {
  const std::string last_chunk = "0" + http::kCrlf + http::kCrlf;
  body_.AppendDataToBuffer(last_chunk);
}

void CgiResponse::AppendBodyFromBuffer(const utils::ByteVector &buffer) {
  body_.insert(body_.end(), buffer.begin(), buffer.end());
}

void CgiResponse::AdjustHeadersBasedOnResponseType() {
  if (response_type_ == kDocumentResponse) {
    if (GetHeader("STATUS").IsErr()) {
      headers_.push_back(HeaderPairType("STATUS", "200 OK"));
    }
  }
}

Result<CgiResponse::HeaderVecType> CgiResponse::GetHeaderVecFromBuffer(
    const utils::ByteVector &buffer) const {
  Result<size_t> headers_boundary_res =
      buffer.FindString(newline_chars_ + newline_chars_);
  if (headers_boundary_res.IsErr()) {
    return Error();
  }
  size_t headers_boundary = headers_boundary_res.Ok();

  std::string headers_str =
      std::string(buffer.begin(), buffer.begin() + headers_boundary);

  std::vector<std::string> header_strs =
      utils::SplitString(headers_str, newline_chars_);
  std::set<std::string> used_headers;
  HeaderVecType headers;
  for (std::vector<std::string>::const_iterator it = header_strs.begin();
       it != header_strs.end(); ++it) {
    Result<std::string> key_res = GetKeyOfHeader(*it);
    Result<std::string> value_res = GetValueOfHeader(*it);
    if (key_res.IsErr() || value_res.IsErr()) {
      return Error();
    }
    std::string key = key_res.Ok();
    std::string value = value_res.Ok();

    // 同じヘッダーが2回現れてはいけない
    if (used_headers.find(key) != used_headers.end()) {
      return Error();
    }
    // ヘッダーとして利用不可能な文字が含まれていないか
    if (!IsValidHeaderKey(key) || !IsValidHeaderValue(value)) {
      return Error();
    }

    headers.push_back(HeaderPairType(key, value));
    used_headers.insert(key);
  }
  return headers;
}

bool CgiResponse::IsDocumentResponse() const {
  if (headers_.size() < 1) {
    return false;
  }
  bool is_valid_content_type = headers_[0].first == "CONTENT-TYPE";
  bool is_valid_status = true;
  if (headers_.size() >= 2 && headers_[1].first == "STATUS") {
    is_valid_status = IsValidStatusHeaderValue(headers_[1].second);
  }
  return is_valid_content_type && is_valid_status;
}

bool CgiResponse::IsLocalRedirectResponse() const {
  return headers_.size() == 1 && headers_[0].first == "LOCATION" &&
         IsLocalPathQuery(headers_[0].second);
}

bool CgiResponse::IsClientRedirectResponse() const {
  return headers_.size() >= 1 && headers_[0].first == "LOCATION" &&
         IsFragmentUri(headers_[0].second);
}

bool CgiResponse::IsClientRedirectResponseWithDocument() const {
  if (headers_.size() < 3) {
    return false;
  }
  bool is_valid_location =
      headers_[0].first == "LOCATION" && IsFragmentUri(headers_[0].second);
  bool is_valid_status = headers_[1].first == "STATUS" &&
                         IsValidStatusHeaderValue(headers_[1].second) &&
                         headers_[1].second[0] == '3';
  bool is_valid_content_type = headers_[2].first == "CONTENT-TYPE";
  return is_valid_location && is_valid_status && is_valid_content_type;
}

bool CgiResponse::IsValidStatusHeaderValue(const std::string &val) const {
  std::string::size_type sp_pos = val.find(" ");
  std::string status = val.substr(0, sp_pos);
  Result<unsigned long> result = utils::Stoul(status);
  if (result.IsErr()) {
    return false;
  }
  return http::StatusCodes::IsHttpStatus(result.Ok());
}

bool CgiResponse::IsComposedOfUriC(const std::string &str) const {
  const char *reserved_marks = ";/?:@&=+$,[]";
  const char *unreserved_marks = "-_.!~*'()";

  std::string::size_type idx = 0;
  while (idx < str.length()) {
    if (isalnum(str[idx]) || strchr(reserved_marks, str[idx]) ||
        strchr(unreserved_marks, str[idx])) {
      ++idx;
    } else if (str[idx] == '%') {
      // percent encoding
      if (str.length() - idx < 3 ||
          utils::Stoul(str.substr(idx + 1, 2), utils::kHexadecimal).IsErr()) {
        return false;
      }
      idx += 3;
    } else {
      return false;
    }
  }
  return true;
}

bool CgiResponse::IsLocalPathQuery(const std::string &pathquery) const {
  if (pathquery.empty() || pathquery[0] != '/') {
    return false;
  }

  const char *unreserved_marks = "-_.!~*'()";
  const char *extra_marks = ":@&=+$,";

  std::string::size_type question_idx = pathquery.find('?');

  // abs-path のチェック
  size_t abs_path_len =
      question_idx != std::string::npos ? question_idx : pathquery.size();
  size_t idx = 0;
  while (idx < abs_path_len) {
    if (isalnum(pathquery[idx]) || strchr(unreserved_marks, pathquery[idx]) ||
        strchr(extra_marks, pathquery[idx]) || pathquery[idx] == '/') {
      // allowed char
      ++idx;
    } else if (pathquery[idx] == '%') {
      // percent encoding
      if (abs_path_len - idx < 3 ||
          utils::Stoul(pathquery.substr(idx + 1, 2), utils::kHexadecimal)
              .IsErr()) {
        return false;
      }
      idx += 3;
    } else {
      // char is not allowed
      return false;
    }
  }

  // query-string のチェック
  if (question_idx != std::string::npos &&
      pathquery.size() - question_idx > 0) {
    std::string query = pathquery.substr(question_idx + 1);
    if (!IsComposedOfUriC(query)) {
      return false;
    }
  }
  return true;
}

bool CgiResponse::IsAbsoluteUri(const std::string &uri) const {
  std::string::size_type scheme_end = uri.find("://");
  if (scheme_end == std::string::npos || scheme_end == 0 ||
      scheme_end + 3 >= uri.size()) {
    return false;
  }
  return true;
}

bool CgiResponse::IsFragmentUri(const std::string &uri) const {
  std::string::size_type sharp_idx = uri.find("#");
  if (sharp_idx != std::string::npos) {
    std::string abs_uri = uri.substr(0, sharp_idx);
    std::string fragment = uri.substr(sharp_idx + 1);
    return IsAbsoluteUri(abs_uri) && IsComposedOfUriC(fragment);
  }
  return IsAbsoluteUri(uri);
}

bool CgiResponse::IsValidHeaderKey(const std::string &key) const {
  std::string::size_type i = 0;
  while (i < key.size()) {
    if (kCharExceptCtlAndSeparator.find(key[i]) == std::string::npos) {
      return false;
    }
    i++;
  }
  return true;
}

bool CgiResponse::IsValidHeaderValue(const std::string &value) const {
  bool is_quoted = false;

  std::string::size_type i = 0;
  while (i < value.size()) {
    if (!is_quoted) {
      if (kCharExceptCtlAndSeparator.find(value[i]) == std::string::npos &&
          kSeparator.find(value[i]) == std::string::npos) {
        return false;
      }
      if (value[i] == '"') {
        is_quoted = true;
      }
    } else {
      if (value[i] == '"') {
        is_quoted = false;
      } else if (kQdtext.find(value[i]) == std::string::npos) {
        return false;
      }
    }
    i++;
  }

  if (is_quoted) {
    return false;
  }
  return true;
}

namespace {

Result<std::string> GetKeyOfHeader(const std::string &header) {
  std::string key;
  std::string::size_type colon_pos = header.find(":");
  if (colon_pos == std::string::npos) {
    return Error();
  } else {
    key = header.substr(0, colon_pos);
  }
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  return key;
}

Result<std::string> GetValueOfHeader(const std::string &header) {
  std::string value;
  std::string::size_type colon_pos = header.find(":");
  if (colon_pos == std::string::npos) {
    return Error();
  } else if (colon_pos + 1 == header.size()) {
    value = "";
  } else {
    std::string::size_type idx = colon_pos + 1;
    while (idx < header.size() && isspace(header[idx])) {
      ++idx;
    }
    value = header.substr(idx);
  }
  return value;
}

}  // namespace

}  // namespace cgi
