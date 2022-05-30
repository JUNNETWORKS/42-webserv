#include <gtest/gtest.h>

#include "config/config_parser.hpp"

namespace config {

namespace {
const int64_t kDefaultClientMaxBodySize = 1024 * 1024;  // 1MB
const std::string kConfigurationDirPath = "unit_test/config/configurations/";
}  // namespace

TEST(ParserTest, FileNameIsInvalid) {
  Parser parser;
  EXPECT_THROW(parser.LoadFile(kConfigurationDirPath + "NotExists.conf");
               , Parser::ParserException);
}

TEST(ParserTest, SimpleServer) {
  Parser parser;
  parser.LoadFile(kConfigurationDirPath + "SimpleServer.conf");
  Config config = parser.ParseConfig();
  config.Print();
  EXPECT_TRUE(config.IsValid());

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  ASSERT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->GetListenPort() == "8080");

  const LocationConf *location = vserver->GetLocation("/");
  ASSERT_TRUE(location != NULL);
  EXPECT_TRUE(location->GetPathPattern() == "/");
  EXPECT_TRUE(location->GetIsBackwardSearch() == false);
  EXPECT_TRUE(location->IsMethodAllowed("GET") == true);
  EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
  EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
  EXPECT_TRUE(location->GetClientMaxBodySize() == kDefaultClientMaxBodySize);
  EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  LocationConf::IndexPagesVector indexpages{"index.html"};
  EXPECT_TRUE(location->GetIndexPages() == indexpages);
  EXPECT_TRUE(location->GetIsCgi() == false);
  LocationConf::ErrorPagesMap errorpages{
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(404),
          "NotFound.html"),
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(403),
          "NotFound.html"),
  };
  EXPECT_TRUE(location->GetErrorPages() == errorpages);
  EXPECT_TRUE(location->GetAutoIndex() == false);
  EXPECT_TRUE(location->GetRedirectUrl() == "");
}

TEST(ParserTest, SimpleServerInOneLine) {
  Parser parser;
  parser.LoadFile(kConfigurationDirPath + "SimpleServerInOneLine.conf");
  Config config = parser.ParseConfig();
  config.Print();
  EXPECT_TRUE(config.IsValid());

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  ASSERT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->GetListenPort() == "8080");

  const LocationConf *location = vserver->GetLocation("/");
  ASSERT_TRUE(location != NULL);
  EXPECT_TRUE(location->GetPathPattern() == "/");
  EXPECT_TRUE(location->GetIsBackwardSearch() == false);
  EXPECT_TRUE(location->IsMethodAllowed("GET") == true);
  EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
  EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
  EXPECT_TRUE(location->GetClientMaxBodySize() == kDefaultClientMaxBodySize);
  EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  LocationConf::IndexPagesVector indexpages{"index.html"};
  EXPECT_TRUE(location->GetIndexPages() ==
              LocationConf::IndexPagesVector{"index.html"});
  LocationConf::ErrorPagesMap errorpages{
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(404),
          "NotFound.html"),
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(403),
          "NotFound.html"),
  };
  EXPECT_TRUE(location->GetErrorPages() == errorpages);
  EXPECT_TRUE(location->GetIsCgi() == false);
  EXPECT_TRUE(location->GetAutoIndex() == false);
  EXPECT_TRUE(location->GetRedirectUrl() == "");
}

TEST(ParserTest, EscapedChar) {
  Parser parser;
  parser.LoadFile(kConfigurationDirPath + "EscapedChar.conf");
  Config config = parser.ParseConfig();
  config.Print();

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  ASSERT_TRUE(vserver != NULL);

  const LocationConf *location = vserver->GetLocation("/");
  ASSERT_TRUE(location != NULL);
  EXPECT_TRUE(location->GetPathPattern() == "/");
  EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  LocationConf::IndexPagesVector indexpages{"index.;ht\\ml"};
  EXPECT_TRUE(location->GetIndexPages() == indexpages);
  EXPECT_TRUE(location->GetRedirectUrl() == "");
  LocationConf::ErrorPagesMap errorpages{
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(404),
          "Not Found.html"),
      std::make_pair<LocationConf::ErrorPagesMap::key_type,
                     LocationConf::ErrorPagesMap::mapped_type>(
          static_cast<LocationConf::ErrorPagesMap::key_type>(403),
          "Not Found.html"),
  };
  EXPECT_TRUE(location->GetErrorPages() == errorpages);
  EXPECT_TRUE(config.IsValid());
}

TEST(ParserTest, MultipleValidServers) {
  Parser parser;
  parser.LoadFile(kConfigurationDirPath + "MultipleValidServers.conf");
  Config config = parser.ParseConfig();
  config.Print();

  {
    const VirtualServerConf *vserver =
        config.GetVirtualServerConf("80", "localhost");
    ASSERT_TRUE(vserver != NULL);

    {
      const LocationConf *location = vserver->GetLocation("/");
      ASSERT_TRUE(location != NULL);
      EXPECT_TRUE(location->GetPathPattern() == "/");
      EXPECT_TRUE(location->GetIsBackwardSearch() == false);
      EXPECT_TRUE(location->IsMethodAllowed("GET") == true);
      EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
      EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
      EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
      LocationConf::IndexPagesVector indexpages = {"index.html", "index.htm"};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages{
          std::make_pair<LocationConf::ErrorPagesMap::key_type,
                         LocationConf::ErrorPagesMap::mapped_type>(
              static_cast<LocationConf::ErrorPagesMap::key_type>(500),
              "server_error_page.html"),
          std::make_pair<LocationConf::ErrorPagesMap::key_type,
                         LocationConf::ErrorPagesMap::mapped_type>(
              static_cast<LocationConf::ErrorPagesMap::key_type>(404),
              "not_found.html"),
          std::make_pair<LocationConf::ErrorPagesMap::key_type,
                         LocationConf::ErrorPagesMap::mapped_type>(
              static_cast<LocationConf::ErrorPagesMap::key_type>(403),
              "not_found.html"),
      };
      EXPECT_TRUE(location->GetErrorPages() == errorpages);
      EXPECT_TRUE(location->GetRedirectUrl() == "");
      EXPECT_TRUE(location->GetAutoIndex() == false);
      EXPECT_TRUE(location->GetIsCgi() == false);
      EXPECT_TRUE(config.IsValid());
    }

    {
      const LocationConf *location = vserver->GetLocation("/upload");
      ASSERT_TRUE(location != NULL);
      EXPECT_TRUE(location->GetPathPattern() == "/upload");
      EXPECT_TRUE(location->GetIsBackwardSearch() == false);
      EXPECT_TRUE(location->IsMethodAllowed("GET") == true);
      EXPECT_TRUE(location->IsMethodAllowed("POST") == true);
      EXPECT_TRUE(location->IsMethodAllowed("DELETE") == true);
      EXPECT_TRUE(location->GetRootDir() == "/var/www/user_uploads");
      LocationConf::IndexPagesVector indexpages{};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages{};
      EXPECT_TRUE(location->GetErrorPages() == errorpages);
      EXPECT_TRUE(location->GetClientMaxBodySize() == 1073741824);
      EXPECT_TRUE(location->GetRedirectUrl() == "");
      EXPECT_TRUE(location->GetAutoIndex() == true);
      EXPECT_TRUE(location->GetIsCgi() == false);
      EXPECT_TRUE(config.IsValid());
    }
  }

  {
    const VirtualServerConf *vserver =
        config.GetVirtualServerConf("80", "www.webserv.com");
    ASSERT_TRUE(vserver != NULL);

    {
      const LocationConf *location = vserver->GetLocation("/");
      ASSERT_TRUE(location != NULL);
      EXPECT_TRUE(location->GetPathPattern() == "/");
      EXPECT_TRUE(location->GetIsBackwardSearch() == false);
      EXPECT_TRUE(location->IsMethodAllowed("GET") == false);
      EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
      EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
      EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
      LocationConf::IndexPagesVector indexpages{"index.html"};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages{};
      EXPECT_TRUE(location->GetErrorPages() == errorpages);
      EXPECT_TRUE(location->GetClientMaxBodySize() ==
                  kDefaultClientMaxBodySize);
      EXPECT_TRUE(location->GetRedirectUrl() == "");
      EXPECT_TRUE(location->GetAutoIndex() == false);
      EXPECT_TRUE(location->GetIsCgi() == false);
      EXPECT_TRUE(config.IsValid());
    }

    {
      const LocationConf *location = vserver->GetLocation(".php");
      ASSERT_TRUE(location != NULL);
      EXPECT_TRUE(location->GetPathPattern() == ".php");
      EXPECT_TRUE(location->GetIsBackwardSearch() == true);
      EXPECT_TRUE(location->IsMethodAllowed("GET") == false);
      EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
      EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
      EXPECT_TRUE(location->GetRootDir() == "/home/nginx/cgi_bins");
      LocationConf::IndexPagesVector indexpages{};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages{};
      EXPECT_TRUE(location->GetErrorPages() == errorpages);
      EXPECT_TRUE(location->GetClientMaxBodySize() ==
                  kDefaultClientMaxBodySize);
      EXPECT_TRUE(location->GetRedirectUrl() == "");
      EXPECT_TRUE(location->GetAutoIndex() == false);
      EXPECT_TRUE(location->GetIsCgi() == true);
      EXPECT_TRUE(config.IsValid());
    }

    EXPECT_TRUE(config.IsValid());
  }

  {
    const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
    ASSERT_TRUE(vserver != NULL);

    const LocationConf *location = vserver->GetLocation("/");
    ASSERT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetPathPattern() == "/");
    EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
    LocationConf::IndexPagesVector indexpages{"index.html"};
    EXPECT_TRUE(location->GetIndexPages() == indexpages);
    LocationConf::ErrorPagesMap errorpages{};
    EXPECT_TRUE(location->GetErrorPages() == errorpages);
    EXPECT_TRUE(location->GetRedirectUrl() == "");
    EXPECT_TRUE(config.IsValid());
  }

  {
    const VirtualServerConf *vserver = config.GetVirtualServerConf("9090", "");
    ASSERT_TRUE(vserver != NULL);
    const LocationConf *location = vserver->GetLocation("/");
    ASSERT_TRUE(location != NULL);
    EXPECT_TRUE(location->GetPathPattern() == "/");
    EXPECT_TRUE(location->GetRootDir() == "");
    LocationConf::IndexPagesVector indexpages{};
    EXPECT_TRUE(location->GetIndexPages() == indexpages);
    LocationConf::ErrorPagesMap errorpages{};
    EXPECT_TRUE(location->GetErrorPages() == errorpages);
    EXPECT_TRUE(location->GetRedirectUrl() == "http://localhost:8080/");
    EXPECT_TRUE(config.IsValid());
  }
}

TEST(ParserTest, ConfigFileIsNotFound) {
  Parser parser;
  EXPECT_THROW(
      parser.LoadFile(kConfigurationDirPath + "ConfigFileIsNotFound.conf"),
      Parser::ParserException);
}

TEST(ParserTest, LocationDoesntHaveRoot) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "                                             "
      "  location / {                               "
      "    allow_method GET;                        "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  Config config = parser.ParseConfig();
  config.Print();

  EXPECT_TRUE(config.IsValid() == false);
}

TEST(ParserTest, ServerDoesntHaveListen) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  location / {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  Config config = parser.ParseConfig();
  config.Print();

  EXPECT_TRUE(config.IsValid() == false);
}

TEST(ParserTest, AllowMethodIsInvalid) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "                                             "
      "  location / {                               "
      "    allow_method GET INVALID;                "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

TEST(ParserTest, PortNumberIsTooBig) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 65536;                              "
      "                                             "
      "  location / {                               "
      "    allow_method GET INVALID;                "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

TEST(ParserTest, PortNumberIsNegative) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen -1;                                 "
      "                                             "
      "  location / {                               "
      "    allow_method GET INVALID;                "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

// ドメイン名のルール: https://www.nic.ad.jp/ja/dom/system.html
//
// server_name の引数のドメイン名が 255 文字｡
TEST(ParserTest, ServerNameDomainIsTooLong) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "  server_name "
      "012345678901234567890123456789012345678901234567890123456789012."
      "012345678901234567890123456789012345678901234567890123456789012."
      "012345678901234567890123456789012345678901234567890123456789012."
      "012345678901234567890123456789012345678901234567890123456789012;"
      "                                             "
      "  location / {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

// ドメインラベルが64文字
TEST(ParserTest, ServerNameDomainLabelIsTooLong) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "  server_name "
      "0123456789012345678901234567890123456789012345678901234567890123.com;"
      "                                             "
      "  location / {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

// ドメインラベルのフォーマットが違う
TEST(ParserTest, ServerNameFormatIsInvalid) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "  server_name -hoge.com;                     "
      "                                             "
      "  location / {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

TEST(ParserTest, LocationPathPatternIsMissing) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "                                             "
      "  location   {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

TEST(ParserTest, LocationBackPathPatternIsMissing) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "                                             "
      "  location_back  {                           "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
}

// autoindex の引数は 'on' || 'off' 以外はエラー
TEST(ParserTest, AutoIndexArgIsInvalid) {
  Parser parser;
  parser.LoadData(
      "server {                                     "
      "  listen 8080;                               "
      "                                             "
      "  location / {                               "
      "    allow_method GET;                        "
      "    root /var/www/html;                      "
      "    index index.html;                        "
      "    error_page 404 403 NotFound.html;        "
      "    autoindex enabled;                       "
      "  }                                          "
      "}                                            ");
  EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
};

TEST(ParserTest, HttpStatusInErrorPagesAreInvalid) {
  {
    // 符号なし整数じゃない
    Parser parser;
    parser.LoadData(
        "server {                                     "
        "  listen 8080;                               "
        "                                             "
        "  location / {                               "
        "    allow_method GET;                        "
        "    root /var/www/html;                      "
        "    index index.html;                        "
        "    error_page hoge NotFound.html;            "
        "  }                                          "
        "}                                            ");
    EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
  }

  {
    // 3桁じゃない
    Parser parser;
    parser.LoadData(
        "server {                                     "
        "  listen 8080;                               "
        "                                             "
        "  location / {                               "
        "    allow_method GET;                        "
        "    root /var/www/html;                      "
        "    index index.html;                        "
        "    error_page 1 NotFound.html;            "
        "  }                                          "
        "}                                            ");
    EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
  }

  {
    // ステータスコードの1桁目が1~5ではない
    Parser parser;
    parser.LoadData(
        "server {                                     "
        "  listen 8080;                               "
        "                                             "
        "  location / {                               "
        "    allow_method GET;                        "
        "    root /var/www/html;                      "
        "    index index.html;                        "
        "    error_page 800 NotFound.html;            "
        "  }                                          "
        "}                                            ");
    EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
  }

  {
    // ステータスコードの1桁目が1~5ではない
    Parser parser;
    parser.LoadData(
        "server {                                     "
        "  listen 8080;                               "
        "                                             "
        "  location / {                               "
        "    allow_method GET;                        "
        "    root /var/www/html;                      "
        "    index index.html;                        "
        "    error_page 000 NotFound.html;            "
        "  }                                          "
        "}                                            ");
    EXPECT_THROW(parser.ParseConfig();, Parser::ParserException);
  }
}

};  // namespace config
