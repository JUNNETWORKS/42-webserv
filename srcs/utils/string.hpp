#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

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

// idx が非NULLだった場合､変換に使用されなかったidxが代入される｡
long long stoll(const std::string &str, size_t *idx = NULL,
                long long base = 10);

}  // namespace utils

#endif
