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

  // configとして正しいか
  bool IsValid() const;

  void Print() const;

  // listen_port と server_name を元に適切なバーチャルサーバを返す｡
  // 該当するバーチャルサーバがない場合はNULLを返す｡
  const VirtualServerConf *GetVirtualServerConf(
      const PortType listen_port, const std::string &server_name) const;

  // すべてのバーチャルサーバを保持するvectorへの参照を返す｡
  const VirtualServerConfVector &GetVirtualServerConfs() const;

  // バーチャルサーバを追加する｡
  //
  // 引数がポインタじゃないのはデータがスタック領域とヒープ領域に混在するのを避けるためである｡
  void AppendVirtualServerConf(const VirtualServerConf &virtual_server_conf);
};

Config ParseConfig(const char *filename);

Config CreateSampleConfig();

};  // namespace config

#endif
