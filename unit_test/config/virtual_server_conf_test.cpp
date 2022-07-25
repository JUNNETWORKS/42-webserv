#include "config/virtual_server_conf.hpp"

#include <gtest/gtest.h>

#include "config/config.hpp"
#include "config/location_conf.hpp"
#include "create_test_config.hpp"

namespace config {

// 前方一致のpathで最長文字数一致にヒットする
TEST(VirtualServerConfTest, GetLocationByForwardMatch) {
  Config config = CreateTestConfig();
  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "localhost");

  {
    const LocationConf *location = vserver->GetLocation("/");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  }
  {
    const LocationConf *location =
        vserver->GetLocation("/users/123/index.html");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  }
  {
    const LocationConf *location = vserver->GetLocation("/upload");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/user_uploads");
  }
  {
    const LocationConf *location =
        vserver->GetLocation("/upload/images/pikachu.jpg");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/user_uploads");
  }
  {
    const LocationConf *location = vserver->GetLocation("/upload2");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/user_uploads2");
  }
  {
    const LocationConf *location =
        vserver->GetLocation("/upload2/images/pikachu.jpg");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/var/www/user_uploads2");
  }
}

// 後方一致のpathで最長文字数一致でヒットする
TEST(VirtualServerConfTest, GetLocationByBackwardMatch) {
  Config config = CreateTestConfig();
  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "www.webserv.com");

  {
    const LocationConf *location = vserver->GetLocation("/index.php");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/home/nginx/cgi_bins");
  }
  {
    const LocationConf *location = vserver->GetLocation("/cgis/users.php");
    EXPECT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetRootDir() == "/home/nginx/cgi_bins");
  }
}

// 最長文字数一致が複数ある場合は先に記述されたものがヒットする
TEST(VirtualServerConfTest, GetFirstLocationIfMultipleServerHaveSamePath) {
  Config config = CreateTestConfig();
  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8888", "localhost");
  const LocationConf *location = vserver->GetLocation("/");
  EXPECT_TRUE(location != NULL);
  EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
}

// どれとも一致しない場合はNULLを返す
TEST(VirtualServerConfTest, ReturnNullIfNoServerIsMatched) {
  Config config = CreateTestConfig();
  const VirtualServerConf *vserver =
      config.GetVirtualServerConf("0.0.0.0", "8080", "localhost");
  const LocationConf *location =
      vserver->GetLocation("hogefuga");  // /が先頭についてない
  EXPECT_TRUE(location == NULL);
}

}  // namespace config
