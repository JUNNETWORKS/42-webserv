#include "utils/path.hpp"

#include <iostream>
#include <vector>

#include "utils/string.hpp"

namespace utils {
static std::vector<std::string> SplitPath(const std::string path) {
  std::vector<std::string> v;
  if (path == "") {
    return v;
  }
  bool is_abs_path = path[0] == '/';
  v = SplitString(path, "/");
  if (is_abs_path) {
    v.insert(v.begin(), "/");
  }
  return v;
}

// そもそも、./ も消すべきでないか。
std::string JoinPath(const std::vector<std::string> &v) {
  std::string joined;
  std::string slash;
  bool is_last_slash = v.back() == "";

  for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end();
       it++) {
    if (*it == "" || *it == ".") {
      continue;
    }
    joined += slash + *it;
    if (*it != "/") {
      slash = "/";
    }
  }
  if (joined != "" && joined[joined.length() - 1] != '/' && is_last_slash) {
    joined += "/";
  }
  return joined;
}

std::string JoinPath(const std::string &s1, const std::string &s2) {
  if (s1 == "") {
    return JoinPath(SplitPath(s2));
  }
  if (s2 == "") {
    return JoinPath(SplitPath(s1));
  }
  return JoinPath(SplitPath(s1 + "/" + s2));
}

bool IsValidPath(const std::string &path) {
  return NormalizePath(path).IsOk();
}

// TODO : 最後のスラッシュ消さないようにするべきか？
// TODO : 消さない場合 /hoge/../ -> /hoge/
//                   /hoge/..  -> /hoge or /hoge/   ...?
Result<std::string> NormalizePath(const std::string &path) {
  std::vector<std::string> vec = SplitPath(path);
  std::vector<std::string> normalize;
  for (std::vector<std::string>::const_iterator it = vec.begin();
       it != vec.end(); it++) {
    if (*it == "" || *it == ".") {
      continue;
    }
    if (*it == "..") {
      if (normalize.size() <= 1) {
        return Error();
      }
      normalize.pop_back();
    } else {
      normalize.push_back(*it);
    }
  }
  return (JoinPath(normalize));
}

}  // namespace utils
