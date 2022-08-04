#ifndef UTILS_LOG_HPP
#define UTILS_LOG_HPP

#include <cstdarg>
#include <cstdio>

#include <iostream>

namespace utils {

void PrintLog(const char *fmt, ...);

void PrintDebugLog(const char *fmt, ...);

}  // namespace utils

#endif
