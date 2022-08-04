#include "time.hpp"

#include <cstdlib>
#include <sys/time.h>

#include <ctime>
#include <iomanip>
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
  s << std::setw(4) << std::setfill('0') << (now->tm_year + 1900) << '/';
  s << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '/';
  s << std::setw(2) << std::setfill('0') << now->tm_mday;
  s << " ";
  s << std::setw(2) << std::setfill('0') << now->tm_hour << ":";
  s << std::setw(2) << std::setfill('0') << now->tm_min << ":";
  s << std::setw(2) << std::setfill('0') << now->tm_sec;
  s << "]";

  return s.str();
}

}  // namespace utils
