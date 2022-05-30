#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <fstream>
#include <sstream>
#include <string>

namespace utils {

// 前方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool ForwardMatch(std::string str, std::string pattern);

// 後方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool BackwardMatch(std::string str, std::string pattern);

bool TryExtractBeforeWhiteSpace(std::string &src, std::string &dest);
std::string TrimWhiteSpace(std::string &str);

bool ReadFile(const std::string &path, std::string &dest);

}  // namespace utils

#endif
