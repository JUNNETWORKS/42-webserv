#ifndef UTILS_PATH_HPP
#define UTILS_PATH_HPP

#include <iostream>
#include <vector>

#include "iostream"
#include "result/result.hpp"

namespace utils {

using namespace result;

std::vector<std::string> PathSplit(const std::string path);
std::string PathJoin(const std::vector<std::string> &v);
std::string PathJoin(const std::string &s1, const std::string &s2);

bool IsValidPath(const std::string &path);
Result<std::string> PathNormalization(const std::string &path);

}  // namespace utils

#endif
