#include "config/config.hpp"

#include <gtest/gtest.h>

#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"
#include "create_test_config.hpp"

namespace config {

// ポートからバーチャルサーバを取得できる
TEST(ConfigTest, GetVirtualServerByPort) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// 同じポートに複数のバーチャルサーバがある場合は最初に記載されたバーチャルサーバを取得する
TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePort) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// 同じポートで複数のバーチャルサーバがあるがserver_nameはどれにも該当しない場合最初に記載されたバーチャルサーバを取得する
TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePortButNoOneMatchTheServerName) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "nothing.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// ポートと server_name が指定された場合は対象のバーチャルサーバを取得する
TEST(ConfigTest, GetVirtualServerByPortAndServerName) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "webserv.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("www.webserv.com"));
}

// 該当するポート番号がない場合はNULLを返す
TEST(ConfigTest, ReturnNullIfThereIsNoCorrespondingPortNumber) {
  Config config = CreateTestConfig();

  EXPECT_TRUE(config.GetVirtualServerConf("0.0.0.0", "10", "") == NULL);
}

}  // namespace config
