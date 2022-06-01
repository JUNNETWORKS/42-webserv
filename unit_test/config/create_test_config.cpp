#include "create_test_config.hpp"

#include "config/config_parser.hpp"

namespace config {

Config CreateTestConfig() {
  Config config;

  // server 1
  VirtualServerConf vserver1;
  vserver1.SetListenPort("8080");
  vserver1.AppendServerName("localhost");

  LocationConf location_v1_1;
  location_v1_1.SetPathPattern("/");
  location_v1_1.AppendAllowedMethod("GET");
  location_v1_1.SetRootDir("/var/www/html");
  location_v1_1.AppendIndexPages("index.html");
  location_v1_1.AppendIndexPages("index.htm");
  location_v1_1.AppendErrorPages(http::SERVER_ERROR, "/server_error_page.html");
  location_v1_1.AppendErrorPages(http::NOT_FOUND, "/not_found.html");
  location_v1_1.AppendErrorPages(http::FORBIDDEN, "/not_found.html");
  vserver1.AppendLocation(location_v1_1);

  LocationConf location_v1_2;
  location_v1_2.SetPathPattern("/upload");
  location_v1_2.AppendAllowedMethod("GET");
  location_v1_2.AppendAllowedMethod("POST");
  location_v1_2.AppendAllowedMethod("DELETE");
  location_v1_2.SetRootDir("/var/www/user_uploads");
  location_v1_2.SetClientMaxBodySize(1048576);
  location_v1_2.SetAutoIndex(true);
  vserver1.AppendLocation(location_v1_2);

  LocationConf location_v1_3;
  location_v1_3.SetPathPattern("/upload2");
  location_v1_3.AppendAllowedMethod("GET");
  location_v1_3.AppendAllowedMethod("POST");
  location_v1_3.AppendAllowedMethod("DELETE");
  location_v1_3.SetRootDir("/var/www/user_uploads2");
  location_v1_3.SetClientMaxBodySize(1048576);
  location_v1_3.SetAutoIndex(true);
  vserver1.AppendLocation(location_v1_3);

  config.AppendVirtualServerConf(vserver1);

  // server 2
  VirtualServerConf vserver2;
  vserver2.SetListenPort("8080");
  vserver2.AppendServerName("www.webserv.com");
  vserver2.AppendServerName("webserv.com");

  LocationConf location_v2_1;
  location_v2_1.SetPathPattern("/");
  location_v2_1.SetRootDir("/var/www/html");
  location_v2_1.AppendIndexPages("index.html");
  vserver2.AppendLocation(location_v2_1);

  LocationConf location_v2_2;
  location_v2_2.SetPathPattern("/ab");
  location_v2_2.AppendAllowedMethod("GET");
  location_v2_2.AppendAllowedMethod("POST");
  location_v2_2.AppendAllowedMethod("DELETE");
  location_v2_2.SetRootDir("/var/www/ab");
  vserver2.AppendLocation(location_v2_2);

  LocationConf location_v2_3;
  location_v2_3.SetPathPattern(".php");
  location_v2_3.SetIsBackwardSearch(true);
  location_v2_3.AppendAllowedMethod("GET");
  location_v2_3.AppendAllowedMethod("POST");
  location_v2_3.AppendAllowedMethod("DELETE");
  location_v2_3.SetIsCgi(true);
  location_v2_3.SetRootDir("/home/nginx/cgi_bins");
  vserver2.AppendLocation(location_v2_3);

  config.AppendVirtualServerConf(vserver2);

  // server 3
  VirtualServerConf vserver3;
  vserver3.SetListenPort("8888");
  vserver3.AppendServerName("localhost");

  LocationConf location_v3_1;
  location_v3_1.SetPathPattern("/");
  location_v3_1.SetRootDir("/var/www/html");
  location_v3_1.AppendIndexPages("index.html");
  vserver3.AppendLocation(location_v3_1);

  LocationConf location_v3_2;
  location_v3_2.SetPathPattern("/");
  location_v3_2.SetRootDir("/var/www/html2");
  location_v3_2.AppendIndexPages("index.html");
  vserver3.AppendLocation(location_v3_2);

  config.AppendVirtualServerConf(vserver3);

  // server 4
  VirtualServerConf vserver4;
  vserver4.SetListenPort("9090");

  LocationConf location_v4_1;
  location_v4_1.SetRedirectUrl("http://localhost:8080/");
  vserver4.AppendLocation(location_v4_1);

  config.AppendVirtualServerConf(vserver4);

  return config;
}
}  // namespace config
