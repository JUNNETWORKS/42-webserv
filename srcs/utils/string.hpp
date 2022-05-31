#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <cerrno>
#include <cstdlib>
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

std::string TrimWhiteSpace(std::string& str);

//文字列をunsigned longに変換する
//成功失敗を返り値で取り、変換結果は引数のresに入れた参照
bool TryStrToUl(const std::string& str, unsigned long& res);

}  // namespace utils

#endif
