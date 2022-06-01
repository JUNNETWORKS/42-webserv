#ifndef UNIT_TEST_CONFIG_CREATE_TEST_CONFIG_HPP_
#define UNIT_TEST_CONFIG_CREATE_TEST_CONFIG_HPP_

#include "config/config.hpp"
#include "config/location_conf.hpp"
#include "config/virtual_server_conf.hpp"

namespace config {

// server {
//   listen 8080;
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
//     client_max_body_dize 1048576;
//
//     root /var/www/user_uploads;
//     autoindex on;
//   }
//
//   location /upload2 {
//     allow_method GET POST DELETE;
//
//     client_max_body_dize 1048576;
//
//     root /var/www/user_uploads2;
//     autoindex on;
//   }
// }
//
// server {
//   listen 8080;
//   server_name www.webserv.com webserv.com;
//
//   location / {
//     root /var/www/html;
//     index index.html;
//   }
//
//   location_back /ab {
//     allow_method GET POST DELETE;
//     root /var/www/ab;
//   }
//
//   location_back .php {
//     allow_method GET POST DELETE;
//     is_cgi on;
//     root /home/nginx/cgi_bins;
//   }
// }
//
// server {
//   listen 8888;
//   server_name localhost;
//
//   location / {
//     root /var/www/html;
//     index index.html;
//   }
//
//   location / {
//     root /var/www/html2;
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
Config CreateTestConfig();

}  // namespace config

#endif
