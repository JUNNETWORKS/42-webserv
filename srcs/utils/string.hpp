#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <string>
#include <vector>

namespace utils {

// 前方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool ForwardMatch(std::string str, std::string pattern);

// 後方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool BackwardMatch(std::string str, std::string pattern);

// std::string を long long に変換したものを返す｡
// idx が非NULLだった場合､変換に使用されなかったidxが代入される｡
long long Stoll(const std::string &str, size_t *idx = NULL,
                long long base = 10);

// str を delim で区切った文字列vectorを返す｡
// e.g. SplitString("a,bc,,d", ",") return ["a", "bc", ,"", "d"]
std::vector<std::string> SplitString(const std::string &str,
                                     const std::string &delim);

std::string TrimWhiteSpace(std::string &str);

}  // namespace utils

#endif
