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

std::string GetKeyOfHeader(const std::string &header);
std::string GetValueOfHeader(const std::string &header);

}  // namespace

CgiResponse::CgiResponse()
    : newline_chars_(""), response_type_(kNotIdentified) {}

CgiResponse::CgiResponse(const CgiResponse &rhs) {
  *this = rhs;
}

CgiResponse &CgiResponse::operator=(const CgiResponse &rhs) {
  if (this != &rhs) {
    response_type_ = rhs.response_type_;
    headers_ = rhs.headers_;
    body_ = rhs.body_;
  }
  return *this;
}

CgiResponse::~CgiResponse() {}

CgiResponse::ResponseType CgiResponse::Parse(utils::ByteVector &buffer) {
  if (DetermineNewlineChars(buffer).IsErr()) {
    return response_type_ = kParseError;
  }

  response_type_ = IdentifyResponseType(buffer);
  if (response_type_ == kParseError) {
    return response_type_;
  }

  if (SetHeadersFromBuffer(buffer).IsErr() ||
      SetBodyFromBuffer(buffer).IsErr()) {
    return response_type_ = kParseError;
  }

  AdjustHeadersBasedOnResponseType();

  return response_type_;
}

// ========================================================================
// Getter and Setter

CgiResponse::ResponseType CgiResponse::GetResponseType() const {
  return response_type_;
}

const std::map<std::string, std::string> &CgiResponse::GetHeaders() {
  return headers_;
}

const utils::ByteVector &CgiResponse::GetBody() {
  return body_;
}

Result<void> CgiResponse::DetermineNewlineChars(utils::ByteVector &buffer) {
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

CgiResponse::ResponseType CgiResponse::IdentifyResponseType(
    utils::ByteVector &buffer) {
  assert(newline_chars_.size() > 0);

  // ヘッダー部の取得
  Result<HeaderVecType> headers_res = GetHeaderVecFromBuffer(buffer);
  if (headers_res.IsErr()) {
    return CgiResponse::kParseError;
  }
  HeaderVecType headers = headers_res.Ok();

  Result<size_t> headers_boundary_res =
      buffer.FindString(newline_chars_ + newline_chars_);
  if (headers_boundary_res.IsErr()) {
    return CgiResponse::kParseError;
  }
  size_t headers_boundary = headers_boundary_res.Ok();

  bool has_body = false;
  if (headers_boundary + (newline_chars_ + newline_chars_).size() !=
      buffer.size()) {
    has_body = true;
  }

  if (IsDocumentResponse(headers, has_body)) {
    return CgiResponse::kDocumentResponse;
  } else if (IsLocalRedirectResponse(headers, has_body)) {
    return CgiResponse::kLocalRedirect;
  } else if (IsClientRedirectResponse(headers, has_body)) {
    return CgiResponse::kClientRedirect;
  } else if (IsClientRedirectResponseWithDocument(headers, has_body)) {
    return CgiResponse::kClientRedirectWithDocument;
  } else {
    return CgiResponse::kParseError;
  }
}

Result<void> CgiResponse::SetHeadersFromBuffer(utils::ByteVector &buffer) {
  Result<HeaderVecType> header_vec_res = GetHeaderVecFromBuffer(buffer);
  if (header_vec_res.IsErr()) {
    return Error();
  }
  HeaderVecType header_vec = header_vec_res.Ok();
  for (HeaderVecType::const_iterator it = header_vec.begin();
       it != header_vec.end(); ++it) {
    // 同じヘッダーが2回現れてはいけない
    if (headers_.find(it->first) != headers_.end()) {
      return Error();
    }
    if (!IsValidHeaderKey(it->first) || !IsValidHeaderValue(it->second)) {
      return Error();
    }
    headers_[it->first] = it->second;
  }
  return Result<void>();
}

Result<void> CgiResponse::SetBodyFromBuffer(utils::ByteVector &buffer) {
  Result<size_t> headers_boundary_res =
      buffer.FindString(newline_chars_ + newline_chars_);
  if (headers_boundary_res.IsErr()) {
    return Error();
  }
  size_t headers_boundary = headers_boundary_res.Ok();

  body_ = utils::ByteVector(buffer.begin() + headers_boundary +
                                (newline_chars_ + newline_chars_).size(),
                            buffer.end());
  return Result<void>();
}

void CgiResponse::AdjustHeadersBasedOnResponseType() {
  if (response_type_ == kDocumentResponse) {
    if (headers_.find("STATUS") == headers_.end()) {
      headers_["STATUS"] = "200 OK";
    }
  }
}

Result<CgiResponse::HeaderVecType> CgiResponse::GetHeaderVecFromBuffer(
    utils::ByteVector &buffer) {
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
  std::vector<std::pair<std::string, std::string> > headers;
  for (std::vector<std::string>::const_iterator it = header_strs.begin();
       it != header_strs.end(); ++it) {
    std::string key = GetKeyOfHeader(*it);
    std::string value = GetValueOfHeader(*it);
    headers.push_back(std::pair<std::string, std::string>(key, value));
  }
  return headers;
}

bool CgiResponse::IsDocumentResponse(const HeaderVecType &headers,
                                     bool has_body) {
  (void)has_body;
  if (headers.size() < 1) {
    return false;
  }
  bool is_valid_content_type = headers[0].first == "CONTENT-TYPE";
  bool is_valid_status = true;
  if (headers.size() >= 2 && headers[1].first == "STATUS") {
    is_valid_status = IsValidStatusHeaderValue(headers[1].second);
  }
  return is_valid_content_type && is_valid_status;
}

bool CgiResponse::IsLocalRedirectResponse(const HeaderVecType &headers,
                                          bool has_body) {
  return !has_body && headers.size() == 1 && headers[0].first == "LOCATION" &&
         IsLocalPathQuery(headers[0].second);
}

bool CgiResponse::IsClientRedirectResponse(const HeaderVecType &headers,
                                           bool has_body) {
  return !has_body && headers.size() >= 1 && headers[0].first == "LOCATION" &&
         IsFragmentUri(headers[0].second);
}

bool CgiResponse::IsClientRedirectResponseWithDocument(
    const HeaderVecType &headers, bool has_body) {
  (void)has_body;
  if (headers.size() < 3) {
    return false;
  }
  bool is_valid_location =
      headers[0].first == "LOCATION" && IsFragmentUri(headers[0].second);
  bool is_valid_status = headers[1].first == "STATUS" &&
                         IsValidStatusHeaderValue(headers[1].second) &&
                         headers[1].second[0] == '3';
  bool is_valid_content_type = headers[2].first == "CONTENT-TYPE";
  return is_valid_location && is_valid_status && is_valid_content_type;
}

bool CgiResponse::IsValidStatusHeaderValue(const std::string &val) {
  std::string::size_type sp_pos = val.find(" ");
  std::string status = val.substr(0, sp_pos);
  Result<unsigned long> result = utils::Stoul(status);
  if (result.IsErr()) {
    return false;
  }
  return http::StatusCodes::IsHttpStatus(result.Ok());
}

bool CgiResponse::IsComposedOfUriC(const std::string &str) {
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

bool CgiResponse::IsLocalPathQuery(const std::string &pathquery) {
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

bool CgiResponse::IsAbsoluteUri(const std::string &uri) {
  std::string::size_type scheme_end = uri.find("://");
  if (scheme_end == std::string::npos || scheme_end == 0 ||
      scheme_end + 3 >= uri.size()) {
    return false;
  }
  return true;
}

bool CgiResponse::IsFragmentUri(const std::string &uri) {
  std::string::size_type sharp_idx = uri.find("#");
  if (sharp_idx != std::string::npos) {
    std::string abs_uri = uri.substr(0, sharp_idx);
    std::string fragment = uri.substr(sharp_idx + 1);
    return IsAbsoluteUri(abs_uri) && IsComposedOfUriC(fragment);
  }
  return IsAbsoluteUri(uri);
}

bool CgiResponse::IsValidHeaderKey(const std::string &key) {
  std::string::size_type i = 0;
  while (i < key.size()) {
    if (kCharExceptCtlAndSeparator.find(key[i]) == std::string::npos) {
      return false;
    }
    i++;
  }
  return true;
}

bool CgiResponse::IsValidHeaderValue(const std::string &value) {
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

std::string GetKeyOfHeader(const std::string &header) {
  std::string key;
  std::string::size_type colon_pos = header.find(":");
  if (colon_pos == std::string::npos) {
    key = header;
  } else {
    key = header.substr(0, colon_pos);
  }
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  return key;
}

std::string GetValueOfHeader(const std::string &header) {
  std::string value;
  std::string::size_type colon_pos = header.find(":");
  if (colon_pos == std::string::npos || colon_pos + 1 == header.size()) {
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
