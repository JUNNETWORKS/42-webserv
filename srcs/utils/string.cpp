#include "utils/string.hpp"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <vector>

namespace utils {

bool ForwardMatch(std::string str, std::string pattern) {
  return str.find(pattern) == 0;
}
bool BackwardMatch(std::string str, std::string pattern) {
  return str.rfind(pattern) == str.length() - pattern.length();
}

int Stoi(const std::string &str, size_t *idx, int base) {
  char *end;
  const char *p = str.c_str();
  long num = std::strtol(p, &end, base);
  if (p == end) {
    throw std::invalid_argument("Stoi");
  }
  if (num < std::numeric_limits<int>::min() ||
      num > std::numeric_limits<int>::max() || errno == ERANGE) {
    throw std::out_of_range("Stoi");
  }
  if (idx != NULL) {
    *idx = static_cast<size_t>(end - p);
  }
  return static_cast<int>(num);
}

bool IsDigits(const std::string &str) {
  if (str.empty()) {
    return false;
  }
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it < '0' || *it > '9') {
      return false;
    }
  }
  return true;
}

bool Stoul(unsigned long &result, const std::string &str) {
  if (!IsDigits(str)) {
    return false;
  }

  char *end;
  const char *p = str.c_str();
  int base = 10;
  errno = 0;
  unsigned long num = std::strtoul(p, &end, base);
  size_t used_char_count = end - p;

  // エラーと判断するもの
  //
  // 先頭に数字や符号がない
  // unsigned long の範囲超えてる
  // すべての文字が数字として認識されなかった
  if (p == end || errno == ERANGE || str.size() != used_char_count) {
    return false;
  }
  result = num;
  return true;
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

std::string TrimString(std::string &str, const std::string &charset) {
  size_t start_pos = str.find_first_not_of(charset);
  size_t end_pos = str.find_last_not_of(charset);
  if (start_pos == std::string::npos)
    str.erase(str.begin(), str.end());
  str.erase(str.begin(), str.begin() + start_pos);
  str.erase(str.begin() + end_pos, str.end());
  return str;
}

bool ReadFile(const std::string &path, std::string &dest) {
  std::ostringstream sstr;
  std::ifstream ifs(path.c_str(), std::ios::binary);

  if (!ifs) {
    return false;
  }
  sstr << ifs.rdbuf();
  dest = sstr.str();
  return true;
}

}  // namespace utils
