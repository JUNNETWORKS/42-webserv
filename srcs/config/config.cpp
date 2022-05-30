#include "config/config.hpp"

#include <stdint.h>
#include <sys/sysinfo.h>

#include <cassert>
#include <iostream>

#include "config/config_parser.hpp"
#include "http/http_status.hpp"

namespace config {

Config::Config() : servers_() {}

Config::Config(const Config &rhs) {
  *this = rhs;
}

Config &Config::operator=(const Config &rhs) {
  if (this != &rhs) {
    servers_ = rhs.servers_;
  }
  return *this;
}

Config::~Config() {}

bool Config::IsValid() const {
  for (VirtualServerConfVector::const_iterator it = servers_.begin();
       it != servers_.end(); ++it) {
    if (!(it->IsValid())) {
      return false;
    }
  }
  return true;
}

void Config::Print() const {
  for (VirtualServerConfVector::const_iterator it = servers_.begin();
       it != servers_.end(); ++it) {
    it->Print();
    std::cout << "\n";
  }
}

const VirtualServerConf *Config::GetVirtualServerConf(
    const PortType listen_port, const std::string &server_name) const {
  const VirtualServerConf *virtual_server_conf = NULL;

  for (VirtualServerConfVector::const_iterator it = servers_.begin();
       it != servers_.end(); ++it) {
    if (it->GetListenPort() == listen_port) {
      if (virtual_server_conf == NULL) {
        virtual_server_conf = &(*it);
      } else if (!(virtual_server_conf->IsServerNameIncluded(server_name)) &&
                 it->IsServerNameIncluded(server_name)) {
        virtual_server_conf = &(*it);
      }
    }
  }

  return virtual_server_conf;
}

const std::vector<VirtualServerConf> &Config::GetVirtualServerConfs() const {
  return servers_;
}

void Config::AppendVirtualServerConf(
    const VirtualServerConf &virtual_server_conf) {
  servers_.push_back(virtual_server_conf);
}

Config ParseConfig(const char *filename) {
  Parser parser;
  parser.LoadFile(filename);
  return parser.ParseConfig();
}

// server {
//   listen 80;
//   server_name localhost;
//
//   location / {
//     allow_method GET;
//
//     root /var/www/html;
//     index index.html index.htm;
//
//     error_page 500 /server_error_page.html;
//     error_page 404 403 /not_found.html;
//   }
//
//   location /upload {
//     allow_method GET POST DELETE;
//
//     client_max_body_dize 1M;
//
//     root /var/www/user_uploads;
//     autoindex on;
//   }
// }
//
// server {
//   listen 80;
//   server_name www.webserv.com webserv.com;
//
//   location / {
//     root /var/www/html;
//     index index.html;
//   }
//
//   location_back .php {
//     is_cgi on;
//     root /home/nginx/cgi_bins;
//   }
// }
//
// server {
//   listen 8080;
//   server_name localhost;
//
//   location / {
//     root /var/www/html;
//     index index.html;
//   }
// }
//
// server {
//   listen 9090;
//
//   location / {
//     return http://localhost:8080/;
//   }
// }
Config CreateSampleConfig() {
  Config config;

  // server 1
  VirtualServerConf vserver1;
  vserver1.SetListenPort("80");
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
  location_v1_2.SetAutoIndex(true);
  vserver1.AppendLocation(location_v1_2);

  config.AppendVirtualServerConf(vserver1);

  // server 2
  VirtualServerConf vserver2;
  vserver2.SetListenPort("80");
  vserver2.AppendServerName("www.webserv.com");
  vserver2.AppendServerName("webserv.com");

  LocationConf location_v2_1;
  location_v2_1.SetPathPattern("/");
  location_v2_1.SetRootDir("/var/www/html");
  location_v2_1.AppendIndexPages("index.html");
  vserver2.AppendLocation(location_v2_1);

  LocationConf location_v2_2;
  location_v2_2.SetPathPattern(".php");
  location_v2_2.SetIsBackwardSearch(true);
  location_v2_2.SetIsCgi(true);
  location_v2_2.SetRootDir("/home/nginx/cgi_bins");
  vserver2.AppendLocation(location_v2_2);

  config.AppendVirtualServerConf(vserver2);

  // server 3
  VirtualServerConf vserver3;
  vserver3.SetListenPort("8080");
  vserver3.AppendServerName("localhost");

  LocationConf location_v3_1;
  location_v3_1.SetPathPattern("/");
  location_v3_1.SetRootDir("/var/www/html");
  location_v3_1.AppendIndexPages("index.html");
  vserver3.AppendLocation(location_v3_1);

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
