#ifndef CONFIG_LOCATION_CONF
#define CONFIG_LOCATION_CONF

#include <map>
#include <set>
#include <string>
#include <vector>

#include "http/http_status.hpp"

namespace config {

// serverディレクティブ内のlocationディレクティブの情報
class LocationConf {
 private:
  std::set<std::string> allow_methods_;
  int64_t client_max_body_size_;
  std::string root_;
  std::vector<std::string> index_;
  bool is_cgi_;
  // errorPages[<status_code>] = <error_page_path>
  std::map<http::HttpStatus, std::string> error_pages_;
  // ディレクトリ内ファイル一覧ページを有効にするかどうか
  bool auto_index_;
  // returnディレクティブで指定されたURL
  std::string redirect_url_;
};

};  // namespace config

#endif
