#ifndef HTTP_CONTENT_TYPES_HPP_
#define HTTP_CONTENT_TYPES_HPP_

#include <map>
#include <string>

namespace http {

// https://developer.mozilla.org/ja/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
class ContentTypes {
 private:
  static const std::map<std::string, std::string> content_types_;

 public:
  static std::string GetContentTypeFromExt(const std::string &extension);

 private:
  ContentTypes();
  ContentTypes(const ContentTypes &rhs);
  ContentTypes &operator=(const ContentTypes &rhs);
  ~ContentTypes();

  static std::map<std::string, std::string> CreateContentTypes();
};

}  // namespace http

#endif
