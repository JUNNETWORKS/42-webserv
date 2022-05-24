#include "config/location_conf.hpp"

#include "utils/string.hpp"

namespace config {

LocationConf::LocationConf() {}

LocationConf::LocationConf(const LocationConf &rhs) {
  *this = rhs;
}

LocationConf &LocationConf::operator=(const LocationConf &rhs) {
  if (this != &rhs) {
    path_pattern_ = rhs.path_pattern_;
    is_backward_search_ = rhs.is_backward_search_;
    allowed_methods_ = rhs.allowed_methods_;
    client_max_body_size_kb_ = rhs.client_max_body_size_kb_;
    root_dir_ = rhs.root_dir_;
    index_pages_ = rhs.index_pages_;
    is_cgi_ = rhs.is_cgi_;
    error_pages_ = rhs.error_pages_;
    auto_index_ = rhs.auto_index_;
    redirect_url_ = rhs.redirect_url_;
  }
  return *this;
}

LocationConf::~LocationConf() {}

std::string LocationConf::GetPathPattern() const {
  return path_pattern_;
}

void LocationConf::SetPathPattern(std::string path_pattern) {
  path_pattern_ = path_pattern;
}

bool LocationConf::GetIsBackwardSearch() const {
  return is_backward_search_;
}

void LocationConf::SetIsBackwardSearch(bool is_backward_search) {
  is_backward_search_ = is_backward_search;
}

bool LocationConf::IsMethodAllowed(std::string method) const {
  return allowed_methods_.find(method) != allowed_methods_.end();
}

void LocationConf::AppendAllowedMethod(std::string method) {
  allowed_methods_.insert(method);
}

int64_t LocationConf::GetClientMaxBodySizeKB() const {
  return client_max_body_size_kb_;
}

void LocationConf::SetClientMaxBodySizeKB(int64_t client_max_body_size_kb) {
  client_max_body_size_kb_ = client_max_body_size_kb;
}

std::string LocationConf::GetRootDir() const {
  return root_dir_;
}

void LocationConf::SetRootDir(std::string root_dir) {
  root_dir_ = root_dir;
}

const std::vector<std::string> &LocationConf::GetIndexPages() const {
  return index_pages_;
}

void LocationConf::AppendIndexPages(std::string filepath) {
  index_pages_.push_back(filepath);
}

bool LocationConf::GetIsCgi() const {
  return is_cgi_;
}

void LocationConf::SetIsCgi(bool is_cgi) {
  is_cgi_ = is_cgi;
}

const std::map<http::HttpStatus, std::string> &LocationConf::GetErrorPages()
    const {
  return error_pages_;
}

void LocationConf::AppendErrorPages(http::HttpStatus status,
                                    std::string filepath) {
  if (error_pages_.find(status) == error_pages_.end()) {
    error_pages_[status] = filepath;
  }
}

bool LocationConf::GetAutoIndex() const {
  return auto_index_;
}

void LocationConf::SetAutoIndex(bool auto_index_is_enabled) {
  auto_index_ = auto_index_is_enabled;
}

std::string LocationConf::GetRedirectUrl() const {
  return redirect_url_;
}

void LocationConf::SetRedirectUrl(std::string redirect_url) {
  redirect_url_ = redirect_url;
}

bool LocationConf::IsMatchPattern(std::string path) const {
  if (is_backward_search_) {
    return utils::BackwardMatch(path, path_pattern_);
  } else {
    return utils::ForwardMatch(path, path_pattern_);
  }
}

};  // namespace config
