#include "utils/string.hpp"

#include <cerrno>
#include <cstdlib>
#include <stdexcept>

namespace utils {

bool ForwardMatch(std::string str, std::string pattern) {
  return str.find(pattern) == 0;
}
bool BackwardMatch(std::string str, std::string pattern) {
  return str.rfind(pattern) == str.length() - pattern.length();
}

long long stoll(const std::string &str, size_t *idx, long long base) {
  const char *p = str.c_str();
  char *end;
  long long x = strtoll(p, &end, base);
  if (p == end) {
    throw std::invalid_argument("stoll");
  }
  if (errno == ERANGE) {
    throw std::out_of_range("stoll");
  }
  if (idx != NULL) {
    *idx = static_cast<size_t>(end - p);
  }
  return x;
}

}  // namespace utils
