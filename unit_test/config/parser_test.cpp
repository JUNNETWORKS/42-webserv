#include <gtest/gtest.h>

#include "config/config_parser.hpp"

namespace config {

namespace {
LocationConf CreateLocationConf(
    const std::string &path_pattern, bool is_backward_search,
    LocationConf::AllowedMethodsSet allowed_methods,
    int64_t client_max_body_size, const std::string &root_dir,
    const LocationConf::IndexPagesVector &index_pages, bool is_cgi,
    const LocationConf::ErrorPagesMap &error_pages, bool auto_index,
    const std::string &reidrect_url) {
  LocationConf location;

  location.SetPathPattern(path_pattern);
  location.SetIsBackwardSearch(is_backward_search);
  location.AppendAllowedMethod(path_pattern);

  return location;
}
}  // namespace

TEST(ParserTest, SimpleServer) {
  Parser parser;
  parser.LoadData(
      "server {"
      "listen 8080;"

      "location / {"
      "allow_method GET;"
      "root /var/www/html;"
      "index index.html;"
      "error_page 404 403 NotFound.html;"
      "}"
      "}");
  Config config = parser.ParseConfig();
  config.Print();
  EXPECT_TRUE(config.IsValid());

  const VirtualServerConf *vserver = config.GetVirtualServerConf("8080", "");
  EXPECT_TRUE(vserver != NULL);
  EXPECT_TRUE(vserver->GetListenPort() == "8080");

  const LocationConf *location = vserver->GetLocation("/");
  EXPECT_TRUE(location != NULL);
  /*
  // CreateLocationConf()
  でlocationを作って､それを比較に使うか､それとも全てベタ書きでテストするか const
  LocationConf expected_location = CreateLocationConf(
      "/", false, LocationConf::AllowedMethodsSet{"GET"}, 1024 * 1024,
      "/var/www/html", LocationConf::IndexPagesVector{"index.html"}, false,
      LocationConf::ErrorPagesMap{
          std::make_pair<LocationConf::ErrorPagesMap::key_type,
                         LocationConf::ErrorPagesMap::mapped_type>(
              static_cast<LocationConf::ErrorPagesMap::key_type>(404),
              "NotFound.html"),
          std::make_pair<LocationConf::ErrorPagesMap::key_type,
                         LocationConf::ErrorPagesMap::mapped_type>(
              static_cast<LocationConf::ErrorPagesMap::key_type>(403),
              "NotFound.html"),
      },
      false, "");
      */
  EXPECT_TRUE(location->GetPathPattern() == "/");
  EXPECT_TRUE(location->GetIsBackwardSearch() == false);
  EXPECT_TRUE(location->IsMethodAllowed("GET") == true);
  EXPECT_TRUE(location->IsMethodAllowed("POST") == false);
  EXPECT_TRUE(location->IsMethodAllowed("DELETE") == false);
  EXPECT_TRUE(location->GetClientMaxBodySize() == 1024 * 1024);  // 1MB
  EXPECT_TRUE(location->GetRootDir() == "/var/www/html");
  EXPECT_TRUE(location->GetIndexPages() ==
              LocationConf::IndexPagesVector{"index.html"});
  EXPECT_TRUE(location->GetIsCgi() == false);
  LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{
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

/*
TEST(ParserTest, EscapedChar) {
  Parser parser;
  parser.LoadData(
      "server {"
      "listen 8080;"

      "location / {"
      "allow_method GET;"
      "root /var/www/h\\/tml;"
      "index index.\\;ht\\\\ml;"
      "error_page 404 403 Not\\ Found.html;"
      "}"
      "}");
  Config config = parser.ParseConfig();
  config.Print();

  EXPECT_TRUE(config.IsValid());
  const LocationConf &location =
      config.GetVirtualServerConfs()[0].GetLocation("/");
  EXPECT_TRUE(location.GetRootDir() == "/var/www/h/tml");
  EXPECT_TRUE(location.GetIndexPages() == "index.;ht\\ml");
}

TEST(ParserTest, EscapedChar) {}

TEST(ParserTest, TwoServerHaveSamePortWithDifferentServername) {}

TEST(ParserTest, LocationDoesntHaveRoot) {}

TEST(ParserTest, ServerDoesntHaveListen) {}

TEST(ParserTest, AllowMethodIsInvalid) {}

TEST(ParserTest, PortNumberIsTooBig) {}

TEST(ParserTest, PortNumberIsNegative) {}

TEST(ParserTest, ServerNameDomainIsTooLong) {}

TEST(ParserTest, ServerNameFormatIsInvalid) {}

TEST(ParserTest, PathPatternIsMissing) {}

TEST(ParserTest, AllowMethodIsInvalid) {}

TEST(ParserTest, HttpStatusInErrorPagesAreInvalid) {}

TEST(ParserTest, HttpStatusInErrorPagesAreInvalid) {}

*/

};  // namespace config
