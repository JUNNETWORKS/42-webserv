#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <fstream>
#include <sstream>
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

// std::string を int に変換したものを返す｡
// 変換がが失敗した場合は例外(std::invalid_argument, std::out_of_range)
//   を投げる｡
int Stoi(const std::string &str, size_t *idx = NULL, int base = 10);

// strが数字のみで構成されているか
bool IsDigits(const std::string &str);

// std::string を unsigned long に変換したものを返す｡
//
// 変換がが失敗した場合はfalseを返す
// 全部数字の文字列以外は失敗する設計｡符号もだめ｡
// e.g. "42hoge", "hoge42", "+42" -> false   "42" -> true
bool Stoul(unsigned long &result, const std::string &str);

// str を delim で区切った文字列vectorを返す｡
// e.g. SplitString("a,bc,,d", ",") return ["a", "bc", ,"", "d"]
std::vector<std::string> SplitString(const std::string &str,
                                     const std::string &delim);

// str の前後からcharsetに含まれる文字を消して返す
// e.g. TrimString("abcHELLOabcbca", "abc"") return HELLO
std::string TrimString(std::string &str, const std::string &charset);

bool ReadFile(const std::string &path, std::string &dest);

}  // namespace utils

#endif
