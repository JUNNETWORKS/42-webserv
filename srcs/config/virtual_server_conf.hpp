#ifndef CONFIG_VIRTUAL_SERVER_CONF_HPP_
#define CONFIG_VIRTUAL_SERVER_CONF_HPP_

#include <stdint.h>

#include <set>
#include <string>
#include <vector>

#include "config/location_conf.hpp"

namespace config {

// port は仕様で 16bit の符号なし整数と決められている｡
typedef uint16_t PortType;

// 仮想サーバーの設定. Nginx の server ブロックに相当.
class VirtualServerConf {
 private:
  PortType listen_port_;
  std::set<std::string> server_names_;
  std::vector<LocationConf> locations_;

 public:
  VirtualServerConf();

  VirtualServerConf(const VirtualServerConf &rhs);

  VirtualServerConf &operator=(const VirtualServerConf &rhs);

  ~VirtualServerConf();

  PortType GetListenPort();

  void SetListenPort(PortType listen_port);

  bool IsServerNameIncluded(std::string server_name);

  void AppendServerName(std::string server_name);

  LocationConf *GetLocation(std::string path);

  void AppendLocation(LocationConf location);
};

};  // namespace config

#endif
