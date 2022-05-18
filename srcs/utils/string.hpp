#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <string>

namespace utils {

bool ForwardMatch(std::string str, std::string pattern);
bool BackwardMatch(std::string str, std::string pattern);

}  // namespace utils

#endif
