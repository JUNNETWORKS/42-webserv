#include "create_test_config.hpp"

#include "config/config_parser.hpp"

namespace config {

Config CreateTestConfig() {
  const std::string test_config =
      "server {                                    "
      "  listen 8080;                              "
      "  server_name localhost;                    "
      "                                            "
      "  location / {                              "
      "    allow_method GET;                       "
      "    root /var/www/html;                     "
      "    index index.html index.htm;             "
      "    error_page 500 /server_error_page.html; "
      "    error_page 404 403 /not_found.html;     "
      "  }                                         "
      "                                            "
      "  location /upload {                        "
      "    allow_method GET POST DELETE;           "
      "    root /var/www/user_uploads;             "
      "    client_max_body_size 1048576;           "
      "    autoindex on;                           "
      "  }                                         "
      "                                            "
      "  location /upload2 {                       "
      "    allow_method GET POST DELETE;           "
      "    root /var/www/user_uploads2;            "
      "    client_max_body_size 1048576;           "
      "    autoindex on;                           "
      "  }                                         "
      "}                                           "
      "                                            "
      "server {                                    "
      "  listen 8080;                              "
      "  server_name www.webserv.com webserv.com;  "
      "                                            "
      "  location / {                              "
      "    root /var/www/html;                     "
      "    index index.html;                       "
      "  }                                         "
      "                                            "
      "  location /ab {                            "
      "    allow_method GET POST DELETE;           "
      "    root /var/www/ab;                       "
      "  }                                         "
      "                                            "
      "  location_back .php {                      "
      "    allow_method GET POST DELETE;           "
      "    is_cgi on;                              "
      "    root /home/nginx/cgi_bins;              "
      "  }                                         "
      "}                                           "
      "                                            "
      "server {                                    "
      "  listen 8888;                              "
      "  server_name localhost;                    "
      "                                            "
      "  location / {                              "
      "    root /var/www/html;                     "
      "    index index.html;                       "
      "  }                                         "
      "                                            "
      "  location / {                              "
      "    root /var/www/html2;                    "
      "    index index.html;                       "
      "  }                                         "
      "}                                           "
      "                                            "
      "server {                                    "
      "  listen 9090;                              "
      "                                            "
      "  location / {                              "
      "    return http://localhost:8080/;          "
      "  }                                         "
      "}                                           "
      "                                            "
      "server {                                    "
      "  listen 127.0.0.1:4545;                    "
      "                                            "
      "  location / {                              "
      "    root /var/www/127_0_0_1_4545;           "
      "  }                                         "
      "}                                           "
      "                                            "
      "server {                                    "
      "  listen 127.0.0.2:4545;                    "
      "                                            "
      "  location / {                              "
      "    root /var/www/127_0_0_2_4545;           "
      "  }                                         "
      "}                                           ";

  Parser parser;
  parser.LoadData(test_config);
  Config config = parser.ParseConfig();

  return config;
}
}  // namespace config
