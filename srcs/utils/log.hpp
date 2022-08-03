#ifndef UTILS_LOG_HPP
#define UTILS_LOG_HPP

#include <stdarg.h>
#include <stdio.h>

#include <iostream>

namespace utils {

void PrintLog(const char *fmt, ...);

void PrintDebugLog(const char *fmt, ...);

}  // namespace utils

#endif
