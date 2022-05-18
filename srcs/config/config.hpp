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
 private:
  int worker_num_;
  std::vector<VirtualServerConf> servers_;

 public:
  Config();

  Config(const Config &rhs);

  Config &operator=(const Config &rhs);

  ~Config();

  int32_t GetWorkerNum();

  void SetWorkerNum(int32_t worker_num);

  const VirtualServerConf *GetVirtualServerConf(const PortType listen_port,
                                                const std::string &server_name);

  void AppendVirtualServerConf(const VirtualServerConf &virtual_server_conf);
};

// TODO: Parserができたら消す｡
Config *GetSampleConfig();

};  // namespace config

#endif
