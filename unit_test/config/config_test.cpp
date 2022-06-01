#include "config/config.hpp"

#include <gtest/gtest.h>

#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"

namespace config {

TEST(ConfigTest, GetVirtualServerByPort) {
  // ポートからバーチャルサーバを取得できる
  Config config = CreateSampleConfig();

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePort) {
  // 同じポートに複数のバーチャルサーバがある場合は最初に記載されたバーチャルサーバを取得する
  Config config = CreateSampleConfig();

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePortButNoOneMatchTheServerName) {
  // 同じポートで複数のバーチャルサーバがあるがserver_nameはどれにも該当しない場合最初に記載されたバーチャルサーバを取得する
  Config config = CreateSampleConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("8080", "nothing.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

TEST(ConfigTest, GetVirtualServerByPortAndServerName) {
  // ポートと server_name が指定された場合は対象のバーチャルサーバを取得する
  Config config = CreateSampleConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("8080", "webserv.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("www.webserv.com"));
}

TEST(ConfigTest, ReturnNullIfThereIsNoCorrespondingPortNumber) {
  // 該当するポート番号がない場合はNULLを返す
  Config config = CreateSampleConfig();

  EXPECT_TRUE(config.GetVirtualServerConf("10", "") == NULL);
}


};  // namespace config
