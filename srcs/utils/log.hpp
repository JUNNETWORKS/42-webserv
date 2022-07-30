#ifndef UTILS_LOG_HPP
#define UTILS_LOG_HPP

#include <iostream>

namespace utils {

template <typename T>
void PrintLog(const T &v1) {
  std::cerr << v1 << std::endl;
}

template <typename T, typename U>
void PrintLog(const T &v1, const U &v2) {
  std::cerr << v1 << " " << v2 << std::endl;
}

template <typename T>
void PrintErrorLog(const T &v1) {
  std::cerr << v1 << std::endl;
}

template <typename T, typename U>
void PrintErrorLog(const T &v1, const U &v2) {
  std::cerr << v1 << " " << v2 << std::endl;
}

template <typename T>
void PrintDebugLog(const T &v1) {
#ifndef DEBUG
  return;
#endif
  std::cerr << v1 << std::endl;
}

template <typename T, typename U>
void PrintDebugLog(const T &v1, const U &v2) {
#ifndef DEBUG
  return;
#endif
  std::cerr << v1 << " " << v2 << std::endl;
}

}  // namespace utils

#endif
