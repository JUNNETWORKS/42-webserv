#ifndef CONFIG_VIRTUAL_SERVER_CONF_HPP_
#define CONFIG_VIRTUAL_SERVER_CONF_HPP_

#include <stdint.h>

#include <set>
#include <string>
#include <vector>

#include "config/location_conf.hpp"

namespace config {

const std::string kAnyIpAddress = "0.0.0.0";

// getaddrinfo()などの標準ライブラリのインターフェースに合わせている｡
// サービス名("http")などは受け付けず､数値での指定のみ
typedef std::string PortType;

// 仮想サーバーの設定. Nginx の server ブロックに相当.
class VirtualServerConf {
 public:
  typedef std::vector<LocationConf> LocationConfsVector;
  typedef std::set<std::string> ServerNamesSet;

 private:
  std::string listen_ip_;
  PortType listen_port_;
  ServerNamesSet server_names_;
  LocationConfsVector locations_;

 public:
  VirtualServerConf();

  VirtualServerConf(const VirtualServerConf &rhs);

  VirtualServerConf &operator=(const VirtualServerConf &rhs);

  ~VirtualServerConf();

  bool IsValid() const;

  void Print() const;

  // ========================================================================
  // Getter and Setter
  // ========================================================================

  std::string GetListenIp() const;
  PortType GetListenPort() const;

  void SetListenIp(std::string listen_ip);
  void SetListenPort(PortType listen_port);

  bool IsServerNameIncluded(std::string server_name) const;

  void AppendServerName(std::string server_name);

  // path を元に適切な LocationConf を返す｡
  // path に該当する LocationConf がない場合はNULLを返す｡
  const LocationConf *GetLocation(std::string path) const;

  void AppendLocation(LocationConf location);
};

}  // namespace config

#endif
