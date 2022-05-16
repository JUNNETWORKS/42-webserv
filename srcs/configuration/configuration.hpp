#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#include <map>
#include <set>
#include <vector>

#include "http/http.hpp"

namespace configuration {

typedef uint64_t PortType;
typedef uint64_t StatusCode;

// serverディレクティブ内のlocationディレクティブの情報
class LocationConf {
 private:
  std::set<std::string> allow_methods_;
  int64_t client_max_body_size_;
  std::string root_;
  std::vector<std::string> index_;
  bool is_cgi_;
  // errorPages[<status_code>] = <error_page_path>
  std::map<StatusCode, std::string> error_pages_;
  // ディレクトリ内ファイル一覧ページを有効にするかどうか
  bool auto_index_;
  // returnディレクティブで指定されたURL
  std::string redirect_url_;
};

// 仮想サーバーの設定. Nginx の server ブロックに相当.
class VirtualServerConf {
 private:
  PortType listen_port_;
  std::string server_name_;

  // locations[<path>] = <location_conf>
  std::map<std::string, LocationConf> locations_;
};

class Configuration {
 private:
  // <port, server_name>
  struct VirtualServerSpecifier {
    PortType listen_port_;
    std::string server_name_;
  };

 private:
  const uint64_t worker_num_;
  std::map<VirtualServerSpecifier, VirtualServerConf> servers_;

 public:
};

};  // namespace configuration

#endif
