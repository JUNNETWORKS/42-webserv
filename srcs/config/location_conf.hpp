#ifndef CONFIG_LOCATION_CONF
#define CONFIG_LOCATION_CONF

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace config {

// serverディレクティブ内のlocationディレクティブの情報
class LocationConf {
 private:
  std::string path_pattern_;
  bool is_backward_search_;
  std::set<std::string> allowed_methods_;
  int64_t client_max_body_size_kb_;
  std::string root_dir_;
  std::vector<std::string> index_pages_;
  bool is_cgi_;
  // errorPages[<status_code>] = <error_page_path>
  std::map<http::HttpStatus, std::string> error_pages_;
  // ディレクトリ内ファイル一覧ページを有効にするかどうか
  bool auto_index_;
  // returnディレクティブで指定されたURL
  std::string redirect_url_;

 public:
  LocationConf();

  LocationConf(const LocationConf &rhs);

  LocationConf &operator=(const LocationConf &rhs);

  ~LocationConf();

  std::string GetPathPattern();

  void SetPathPattern(std::string path_pattern);

  bool GetIsBackwardSearch();

  void SetIsBackwardSearch(bool is_backward_search);

  bool IsMethodAllowed(std::string method);

  void AppendAllowedMethod(std::string method);

  int64_t GetClientMaxBodySizeKB();

  void SetClientMaxBodySizeKB(int64_t client_max_body_size_kb);

  std::string GetRootDir();

  void SetRootDir(std::string root_dir);

  const std::vector<std::string> &GetIndexPages();

  void AppendIndexPages(std::string filepath);

  bool GetIsCgi();

  void SetIsCgi(bool is_cgi);

  const std::map<http::HttpStatus, std::string> &GetErrorPages();

  void AppendErrorPages(http::HttpStatus status, std::string filepath);

  bool GetAutoIndex();

  void SetAutoIndex(bool auto_index_is_enabled);

  std::string GetRedirectUrl();

  void SetRedirectUrl(std::string redirect_url);

  bool IsMatchPattern(std::string path);
};

};  // namespace config

#endif
