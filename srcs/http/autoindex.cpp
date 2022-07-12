#include <sys/stat.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "http/http_response.hpp"
#include "result/result.hpp"
#include "utils/File.hpp"
#include "utils/io.hpp"

namespace http {

std::string AutoIndexHead(const std::string &relative_path) {
  std::string head =
      "<html>\n"
      "<head><title>Index of " +
      relative_path +
      "</title></head>\n"
      "<body>\n"
      "<h1>Index of " +
      relative_path + "</h1><hr><pre><a href=\"../\">../</a>\n";
  return head;
}

std::string AutoIndexTail() {
  std::string tail =
      "</pre><hr></body>\n"
      "</html>\n";
  return tail;
}

std::string AutoIndexFileName(const std::string &file_name) {
  std::stringstream ss;

  std::string hyperlink_file_name = file_name;
  std::string display_file_name = file_name;

  if (display_file_name.length() > 50) {
    display_file_name = display_file_name.substr(0, 47) + ".." + "&gt;";
  }

  ss << "<a href=\"" << hyperlink_file_name << "\">";
  ss << display_file_name << "</a>";
  if (display_file_name.length() < 50) {
    ss << std::setw(50 - display_file_name.length()) << "";
  }
  return ss.str();
}

Result<std::string> HttpResponse::MakeAutoIndex(
    const std::string &abs_path, const std::string &relative_path) {
  Result<std::vector<utils::File> > result = utils::GetFileList(abs_path);
  if (result.IsErr()) {
    return Error();
  }

  std::vector<utils::File> file_vec = result.Ok();
  std::sort(file_vec.begin(), file_vec.end());

  std::string autoindex_body;
  for (size_t i = 0; i < file_vec.size(); i++) {
    if (file_vec[i].GetFileName() == "./" ||
        file_vec[i].GetFileName() == "../") {
      continue;
    }
    std::stringstream ss;
    ss << AutoIndexFileName(file_vec[i].GetFileName()) << " ";
    ss << file_vec[i].GetDateStr("%d-%b-%Y %H:%M");
    ss << std::setw(20) << file_vec[i].GetFileSizeStr();
    autoindex_body += ss.str() + "\n";
  }

  return AutoIndexHead(relative_path) + autoindex_body + AutoIndexTail();
}

}  // namespace http
