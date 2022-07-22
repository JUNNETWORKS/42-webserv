#ifndef CONFIG_LOCATION_CONF
#define CONFIG_LOCATION_CONF

#include <stdint.h>

#include <climits>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace config {

// serverディレクティブ内のlocationディレクティブの情報
class LocationConf {
 public:
  typedef std::set<std::string> AllowedMethodsSet;
  typedef std::vector<std::string> IndexPagesVector;
  // errorPages[<status_code>] = <error_page_path>
  typedef std::map<http::HttpStatus, std::string> ErrorPagesMap;

 private:
  std::string path_pattern_;
  bool is_backward_search_;
  AllowedMethodsSet allowed_methods_;
  unsigned long client_max_body_size_;
  std::string root_dir_;
  IndexPagesVector index_pages_;
  bool is_cgi_;
  ErrorPagesMap error_pages_;
  // ディレクトリ内ファイル一覧ページを有効にするかどうか
  bool auto_index_;
  // returnディレクティブで指定されたURL
  std::string redirect_url_;

  static const unsigned long kDefaultClientMaxBodySize = 1024 * 1024;  // 1MB
  static const unsigned long kMaxClientMaxBodySize = INT_MAX;          // 約2GB

 public:
  LocationConf();

  LocationConf(const LocationConf &rhs);

  LocationConf &operator=(const LocationConf &rhs);

  ~LocationConf();

  // ========================================================================
  // Validator
  // ========================================================================

  bool IsValid() const;

  void Print() const;

  // ========================================================================
  // Getter and Setter
  // ========================================================================

  std::string GetPathPattern() const;

  void SetPathPattern(std::string path_pattern);

  bool GetIsBackwardSearch() const;

  void SetIsBackwardSearch(bool is_backward_search);

  bool IsMethodAllowed(std::string method) const;

  void AppendAllowedMethod(std::string method);

  unsigned long GetClientMaxBodySize() const;

  void SetClientMaxBodySize(unsigned long client_max_body_size);

  std::string GetRootDir() const;

  void SetRootDir(std::string root_dir);

  const std::vector<std::string> &GetIndexPages() const;

  void AppendIndexPages(std::string filepath);

  bool GetIsCgi() const;

  void SetIsCgi(bool is_cgi);

  const std::map<http::HttpStatus, std::string> &GetErrorPages() const;

  void AppendErrorPages(http::HttpStatus status, std::string filepath);

  bool GetAutoIndex() const;

  void SetAutoIndex(bool auto_index_is_enabled);

  std::string GetRedirectUrl() const;

  void SetRedirectUrl(std::string redirect_url);

  bool IsMatchPattern(std::string path) const;

  // location : /cgi-bin
  // root dir : /public/cgi-bin
  // req      : /cgi-bin/test-cgi;
  // -> /public/cgi-bin/test-cgi
  std::string GetAbsolutePath(std::string path) const;

  // path     : /cgi-bin/test-cgi/hoge/fuga
  // location : /cgi-bin
  // -> /test-cgi/hoge/fuga
  std::string RemovePathPatternFromPath(std::string path) const;
};

}  // namespace config

#endif
