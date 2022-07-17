#ifndef UTILS_PATH_HPP
#define UTILS_PATH_HPP

#include <iostream>
#include <vector>

#include "iostream"
#include "result/result.hpp"

namespace utils {

using namespace result;

// "a", "b" -> "a/b"
// "a/b/c", "x/y/z" -> "a/b/c/x/y/z"
// "a///b///c", "x/./y/./z" -> "a/b/c/x/y/z"
std::string JoinPath(const std::vector<std::string> &v);
std::string JoinPath(const std::string &s1, const std::string &s2);

// "/hoge/fuga/.." -> "/hoge"
// "/hoge/fuga/../../.." -> Error
bool IsValidPath(const std::string &path);
Result<std::string> NormalizePath(const std::string &path);

// 絶対パスかどうか
bool IsAbsolutePath(const std::string &path);

}  // namespace utils

#endif
