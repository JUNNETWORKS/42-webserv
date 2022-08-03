#include "time.hpp"

#include <stdlib.h>
#include <sys/time.h>

#include <ctime>
#include <iostream>
#include <sstream>

namespace utils {

long GetCurrentTimeMs() {
  timeval tv;

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

// [2021/08/03 10:11:20]
std::string GetDateStr() {
  std::time_t t = time(NULL);
  std::tm* now = std::localtime(&t);

  std::stringstream s;
  s << "[";
  s << (now->tm_year + 1900) << '/' << (now->tm_mon + 1) << '/' << now->tm_mday;
  s << " ";
  s << now->tm_hour << ":" << now->tm_min;
  s << "]";

  return s.str();
}

}  // namespace utils
