#include "utils/string.hpp"

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace utils {

bool ForwardMatch(std::string str, std::string pattern) {
  return str.find(pattern) == 0;
}
bool BackwardMatch(std::string str, std::string pattern) {
  return str.rfind(pattern) == str.length() - pattern.length();
}

long long Stoll(const std::string &str, size_t *idx, long long base) {
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

std::vector<std::string> SplitString(const std::string &str,
                                     const std::string &delim) {
  std::vector<std::string> strs;
  size_t start_idx;
  size_t end_idx;

  start_idx = 0;
  while ((end_idx = str.find(delim, start_idx)) != std::string::npos) {
    end_idx = str.find(delim, start_idx);
    strs.push_back(str.substr(start_idx, end_idx - start_idx));
    start_idx = end_idx + 1;
  }
  // 最後に文字が余っている場合
  if (start_idx < str.length() - 1) {
    strs.push_back(str.substr(start_idx, str.length() - start_idx));
  }
  return strs;
}

}  // namespace utils
