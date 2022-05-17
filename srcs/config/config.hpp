#ifndef CONFIG_CONFIG_HPP
#define CONFIG_CONFIG_HPP

#include <map>
#include <set>
#include <vector>

#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"
#include "http/http.hpp"

namespace config {

class Config {
 private:
  // <port, server_name>
  struct VirtualServerSpecifier {
    const PortType listen_port_;
    const std::string server_name_;
  };

  int32_t worker_num_;
  std::map<VirtualServerSpecifier, VirtualServerConf> servers_;

 public:
  Config();

  Config(const Config &rhs);

  Config &operator=(const Config &rhs);

  ~Config();

  int32_t GetWorkerNum();

  void SetWorkerNum(int32_t worker_num);

  const VirtualServerConf &GetVirtualServerConf(const PortType listen_port,
                                                const std::string &server_name);

  void SetVirtualServerConf(const PortType listen_port,
                            const std::string &server_name,
                            const VirtualServerConf &virtual_server_conf);
};

// TODO: Parserができたら消す｡
Config *GetSampleConfig();

};  // namespace config

#endif
