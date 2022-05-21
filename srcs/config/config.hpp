#ifndef CONFIG_CONFIG_HPP
#define CONFIG_CONFIG_HPP

#include <stdint.h>

#include <map>
#include <set>
#include <vector>

#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"

namespace config {

class Config {
 public:
  typedef std::vector<VirtualServerConf> VirtualServerConfVector;

 private:
  VirtualServerConfVector servers_;

 public:
  Config();

  Config(const Config &rhs);

  Config &operator=(const Config &rhs);

  ~Config();

  const VirtualServerConf &GetVirtualServerConf(
      const PortType listen_port, const std::string &server_name) const;

  const VirtualServerConfVector &GetVirtualServerConfs() const;

  void AppendVirtualServerConf(const VirtualServerConf &virtual_server_conf);
};

// TODO: Parserができたら消す｡
Config CreateSampleConfig();

};  // namespace config

#endif
