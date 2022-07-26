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
      config.GetVirtualServerConf(kAnyIpAddress, "8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// 同じポートに複数のバーチャルサーバがある場合は最初に記載されたバーチャルサーバを取得する
TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePort) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf(kAnyIpAddress, "8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// 同じポートで複数のバーチャルサーバがあるがserver_nameはどれにも該当しない場合最初に記載されたバーチャルサーバを取得する
TEST(
    ConfigTest,
    GetTheFirstVirtualServerListedIfThereAreMultipleVirtualServerOnTheSamePortButNoOneMatchTheServerName) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf(kAnyIpAddress, "8080", "nothing.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("localhost"));
}

// ポートと server_name が指定された場合は対象のバーチャルサーバを取得する
TEST(ConfigTest, GetVirtualServerByPortAndServerName) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver =
      config.GetVirtualServerConf(kAnyIpAddress, "8080", "webserv.com");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->IsServerNameIncluded("www.webserv.com"));
}

// 該当するポート番号がない場合はNULLを返す
TEST(ConfigTest, ReturnNullIfThereIsNoCorrespondingPortNumber) {
  Config config = CreateTestConfig();

  EXPECT_TRUE(config.GetVirtualServerConf(kAnyIpAddress, "10", "") == NULL);
}

// Host:Ip のように指定されているときに正しく取得できる
TEST(ConfigTest, GetVirtualServerByHostIp) {
  Config config = CreateTestConfig();

  const VirtualServerConf *vserver = NULL;

  vserver = config.GetVirtualServerConf("127.0.0.1", "4545", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_EQ(vserver->GetLocation("/")->GetRootDir(), "/var/www/127_0_0_1_4545");

  vserver = config.GetVirtualServerConf("127.0.0.2", "4545", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_EQ(vserver->GetLocation("/")->GetRootDir(), "/var/www/127_0_0_2_4545");

  vserver = config.GetVirtualServerConf("127.0.0.3", "4545", "");
  EXPECT_TRUE(vserver == NULL);
}

}  // namespace config
