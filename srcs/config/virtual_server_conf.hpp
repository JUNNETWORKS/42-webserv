#ifndef VIRTUAL_SERVER_CONF_HPP_
#define VIRTUAL_SERVER_CONF_HPP_

#include <string>

#include "config/location_conf.hpp"

namespace config {

// port は仕様で 16bit の符号なし整数と決められている｡
typedef uint16_t PortType;

// 仮想サーバーの設定. Nginx の server ブロックに相当.
class VirtualServerConf {
 private:
  struct LocationTuple {
    bool is_backward_match_;
    std::string path_;
    LocationConf location_;
  };

  PortType listen_port_;
  std::string server_name_;

  std::vector<LocationTuple> locations;
};

};  // namespace config

#endif
