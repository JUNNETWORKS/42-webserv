#include "config/config.hpp"

#include <stdint.h>
#include <sys/sysinfo.h>

#include <cassert>

namespace config {

Config::Config() : worker_num_(get_nprocs()) {
  assert(worker_num_ > 0);
}

Config::Config(const Config &rhs) {
  *this = rhs;
}

Config &Config::operator=(const Config &rhs) {
  if (this != &rhs) {
    worker_num_ = rhs.worker_num_;
    servers_ = rhs.servers_;
  }
  return *this;
}

Config::~Config() {}

int32_t Config::GetWorkerNum() {
  return worker_num_;
}

void Config::SetWorkerNum(int32_t worker_num) {
  worker_num_ = worker_num;
}

const VirtualServerConf *Config::GetVirtualServerConf(
    const PortType listen_port, const std::string &server_name) {
  VirtualServerConf *virtual_server_conf = NULL;

  for (std::vector<VirtualServerConf>::iterator it = servers_.begin();
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

void Config::AppendVirtualServerConf(
    const VirtualServerConf &virtual_server_conf) {
  servers_.push_back(virtual_server_conf);
}

// server {
//   listen 80;
//   server_name localhost;
//
//   location / {
//     allow_method GET
//
//     root /var/www/html;
//     index index.html index.htm;
//
//     error_page 500 /server_error_page.html
//     error_page 404 403 /not_found.html
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
//     return http://localhost:8080/
//   }
// }
Config *GetSampleConfig() {
  Config *config = new Config();
}

}  // namespace config
