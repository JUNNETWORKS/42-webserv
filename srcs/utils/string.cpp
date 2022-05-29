#include "utils/string.hpp"

namespace utils {

bool ForwardMatch(std::string str, std::string pattern) {
  return str.find(pattern) == 0;
}
bool BackwardMatch(std::string str, std::string pattern) {
  return str.rfind(pattern) == str.length() - pattern.length();
}

bool TryExtractBeforeWhiteSpace(std::string &src, std::string &dest) {
  size_t white_space_pos = src.find_first_of(" ");
  if (white_space_pos == std::string::npos) {
    return false;
  }
  dest = src.substr(0, white_space_pos);
  src.erase(0, white_space_pos + 1);
  return true;
}

std::string TrimWhiteSpace(std::string &str) {
  size_t start_pos = str.find_first_not_of(" ");
  size_t end_pos = str.find_last_not_of(" ");
  if (start_pos == std::string::npos)
    str.erase(str.begin(), str.end());
  str.erase(str.begin(), str.begin() + start_pos);
  str.erase(str.begin() + end_pos, str.end());
  return str;
}

bool readFile(const std::string &path, std::string &dest) {
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
