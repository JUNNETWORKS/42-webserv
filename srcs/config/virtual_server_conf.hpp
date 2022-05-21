#ifndef CONFIG_VIRTUAL_SERVER_CONF_HPP_
#define CONFIG_VIRTUAL_SERVER_CONF_HPP_

#include <stdint.h>

#include <set>
#include <string>
#include <vector>

#include "config/location_conf.hpp"

namespace config {

// getaddrinfo()などの標準ライブラリのインターフェースに合わせている｡
// サービス名("http")などは受け付けず､数値での指定のみ
typedef std::string PortType;

// 仮想サーバーの設定. Nginx の server ブロックに相当.
class VirtualServerConf {
 public:
  typedef std::vector<LocationConf> LocationConfVector;

 private:
  PortType listen_port_;
  std::set<std::string> server_names_;
  LocationConfVector locations_;

 public:
  VirtualServerConf();

  VirtualServerConf(const VirtualServerConf &rhs);

  VirtualServerConf &operator=(const VirtualServerConf &rhs);

  ~VirtualServerConf();

  PortType GetListenPort() const;

  void SetListenPort(PortType listen_port);

  bool IsServerNameIncluded(std::string server_name) const;

  void AppendServerName(std::string server_name);

  const LocationConf &GetLocation(std::string path) const;

  void AppendLocation(LocationConf location);
};

};  // namespace config

#endif
