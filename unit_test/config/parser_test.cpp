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
  EXPECT_TRUE(location->GetIndexPages() ==
              LocationConf::IndexPagesVector{"index.;ht\\ml"});
  EXPECT_TRUE(location->GetRedirectUrl() == "");
  LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{
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
      LocationConf::IndexPagesVector indexpages =
          LocationConf::IndexPagesVector{"index.html", "index.htm"};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{
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
      LocationConf::IndexPagesVector indexpages =
          LocationConf::IndexPagesVector{};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{};
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
      LocationConf::IndexPagesVector indexpages =
          LocationConf::IndexPagesVector{"index.html"};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{};
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
      LocationConf::IndexPagesVector indexpages =
          LocationConf::IndexPagesVector{};
      EXPECT_TRUE(location->GetIndexPages() == indexpages);
      LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{};
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
    LocationConf::IndexPagesVector indexpages =
        LocationConf::IndexPagesVector{"index.html"};
    EXPECT_TRUE(location->GetIndexPages() == indexpages);
    LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{};
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
    LocationConf::IndexPagesVector indexpages =
        LocationConf::IndexPagesVector{};
    EXPECT_TRUE(location->GetIndexPages() == indexpages);
    LocationConf::ErrorPagesMap errorpages = LocationConf::ErrorPagesMap{};
    EXPECT_TRUE(location->GetErrorPages() == errorpages);
    EXPECT_TRUE(location->GetRedirectUrl() == "http://localhost:8080/");
    EXPECT_TRUE(config.IsValid());
  }
}

/*

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

TEST(ParserTest, HttpStatusInErrorPagesAreInvalid){}

*/

};  // namespace config