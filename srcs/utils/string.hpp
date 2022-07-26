#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ByteVector.hpp"
#include "result/result.hpp"

namespace utils {

enum BaseDigit { kDecimal = 10, kHexadecimal = 16 };

using namespace result;

// 前方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool ForwardMatch(std::string str, std::string pattern);

// 後方一致｡
// e.g. str="/upload/icon.jpg",  pattern="/upload/" then return true
//      str="/static/style.css", pattern="/upload/" then return false
bool BackwardMatch(std::string str, std::string pattern);

template <typename T>
std::string ConvertToStr(const T &val) {
  std::stringstream ss;
  ss << val;
  return ss.str();
}

// std::string を int に変換したものを返す｡
// 変換がが失敗した場合は例外(std::invalid_argument, std::out_of_range)
//   を投げる｡
int Stoi(const std::string &str, size_t *idx = NULL, int base = 10);

// strが数字のみで構成されているか
bool IsDigits(const std::string &str);

// strが数字とa~f,A~Fで構成されているか
bool IsHexadecimals(const std::string &str);

// std::string を unsigned long に変換したものを返す｡
//
// 変換がが失敗した場合はfalseを返す
// 全部数字の文字列以外は失敗する設計｡符号もだめ｡
// e.g. "42hoge", "hoge42", "+42" -> false   "42" -> true
Result<unsigned long> Stoul(const std::string &str, BaseDigit base = kDecimal);

std::string PercentEncode(const utils::ByteVector &to_encode);

Result<std::string> PercentDecode(const utils::ByteVector &to_encode);

// str を delim で区切った文字列vectorを返す｡
// e.g. SplitString("a,bc,,d", ",") return ["a", "bc", ,"", "d"]
std::vector<std::string> SplitString(const std::string &str,
                                     const std::string &delim);

// str の前後からcharsetに含まれる文字を消して返す
// e.g. TrimString("abcHELLOabcbca", "abc"") return HELLO
std::string TrimString(std::string &str, const std::string &charset);

bool ReadFile(const std::string &path, std::string &dest);

std::string GetExetension(const std::string &file_path);


std::string ReplaceAll(const std::string s, const std::string &target,
                       const std::string &replacement);

char **AllocVectorStringToCharDptr(const std::vector<std::string> &v);
void DeleteCharDprt(char **dstr);

}  // namespace utils

#endif
