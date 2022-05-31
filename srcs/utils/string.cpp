#include "utils/string.hpp"

namespace utils {

bool ForwardMatch(std::string str, std::string pattern) {
  return str.find(pattern) == 0;
}
bool BackwardMatch(std::string str, std::string pattern) {
  return str.rfind(pattern) == str.length() - pattern.length();
}

std::string TrimWhiteSpace(std::string& str) {
  size_t start_pos = str.find_first_not_of(" ");
  size_t end_pos = str.find_last_not_of(" ");
  if (start_pos == std::string::npos)
    str.erase(str.begin(), str.end());
  str.erase(str.begin(), str.begin() + start_pos);
  str.erase(str.begin() + end_pos, str.end());
  return str;
}

bool TryStrToUl(const std::string& str, unsigned long& res) {
  char* end = NULL;

  errno = 0;
  res = std::strtoul(str.c_str(), &end, 10);
  if (errno != 0 || *end != '\0')
    return false;
  return true;
}
}  // namespace utils
