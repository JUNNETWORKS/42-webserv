#include "http/content_types.hpp"

#include <iostream>

namespace http {

const std::map<std::string, std::string> ContentTypes::content_types_ =
    ContentTypes::CreateContentTypes();

std::string ContentTypes::GetContentTypeFromExt(const std::string &extension) {
  if (content_types_.find(extension) != content_types_.end()) {
    return content_types_.at(extension);
  }
  return "application/octet-stream";
}

std::map<std::string, std::string> ContentTypes::CreateContentTypes() {
  std::map<std::string, std::string> content_types;
  content_types["aac"] = "audio/aac";
  content_types["abw"] = "application/x-abiword";
  content_types["arc"] = "application/x-freearc";
  content_types["avi"] = "video/x-msvideo";
  content_types["azw"] = "application/vnd.amazon.ebook";
  content_types["bin"] = "application/octet-stream";
  content_types["bmp"] = "image/bmp";
  content_types["bz"] = "application/x-bzip";
  content_types["bz2"] = "application/x-bzip2";
  content_types["csh"] = "application/x-csh";
  content_types["css"] = "text/css";
  content_types["csv"] = "text/csv";
  content_types["doc"] = "application/msword";
  content_types["docx"] =
      "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
  content_types["eot"] = "application/vnd.ms-fontobject";
  content_types["epub"] = "application/epub+zip";
  content_types["gz"] = "application/gzip";
  content_types["gif"] = "image/gif";
  content_types["htm"] = "text/html";
  content_types["html"] = "text/html";
  content_types["ico"] = "image/vnd.microsoft.icon";
  content_types["ics"] = "text/calendar";
  content_types["jar"] = "Java Archive (JAR)";
  content_types["jpeg"] = "image/jpeg";
  content_types["jpg"] = "image/jpeg";
  content_types["js"] = "text/javascript";
  content_types["json"] = "application/json";
  content_types["jsonld"] = "application/ld+json";
  content_types["midi"] = "audio/x-midi";
  content_types["mid"] = "audio/midi";
  content_types["mjs"] = "text/javascript";
  content_types["mp3"] = "audio/mpeg";
  content_types["mpeg"] = "video/mpeg";
  content_types["mpkg"] = "application/vnd.apple.installer+xml";
  content_types["odp"] = "application/vnd.oasis.opendocument.presentation";
  content_types["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
  content_types["odt"] = "application/vnd.oasis.opendocument.text";
  content_types["oga"] = "audio/ogg";
  content_types["ogv"] = "video/ogg";
  content_types["ogx"] = "application/ogg";
  content_types["opus"] = "audio/opus";
  content_types["otf"] = "font/otf";
  content_types["png"] = "image/png";
  content_types["pdf"] = "application/pdf";
  content_types["php"] = "application/x-httpd-php";
  content_types["ppt"] = "application/vnd.ms-powerpoint";
  content_types["pptx"] =
      "application/"
      "vnd.openxmlformats-officedocument.presentationml.presentation";
  content_types["rar"] = "application/vnd.rar";
  content_types["rtf"] = "application/rtf";
  content_types["sh"] = "application/x-sh";
  content_types["svg"] = "image/svg+xml";
  content_types["swf"] = "application/x-shockwave-flash";
  content_types["tar"] = "application/x-tar";
  content_types["tif"] = "image/tiff";
  content_types["tiff"] = "image/tiff";
  content_types["ts"] = "video/mp2t";
  content_types["ttf"] = "font/ttf";
  content_types["txt"] = "text/plain";
  content_types["vsd"] = "application/vnd.visio";
  content_types["wav"] = "audio/wav";
  content_types["weba"] = "audio/webm";
  content_types["webm"] = "video/webm";
  content_types["webp"] = "image/webp";
  content_types["woff"] = "font/woff";
  content_types["woff2"] = "font/woff2";
  content_types["xhtml"] = "application/xhtml+xml";
  content_types["xls"] = "application/vnd.ms-excel";
  content_types["xlsx"] =
      "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  content_types["xml"] = "application/xml";
  content_types["xml"] = "text/xml";
  content_types["xul"] = "application/vnd.mozilla.xul+xml";
  content_types["zip"] = "application/zip";
  content_types["3gp"] = "video/3gpp";
  content_types["3gp"] = "audio/3gpp";
  content_types["3g2"] = "video/3gpp2";
  content_types["3g2"] = "audio/3gpp2";
  content_types["7z"] = "application/x-7z-compressed";

  return content_types;
}

}  // namespace http
