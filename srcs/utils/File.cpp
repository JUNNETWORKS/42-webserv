#include "File.hpp"

#include <sys/stat.h>

#include <cassert>
#include <sstream>

namespace utils {

File::File(const std::string& absolute_path)
    : absolute_path_(absolute_path), is_loaded_(false) {}

File::File(const File& rhs) {
  *this = rhs;
}

File::~File() {}

File& File::operator=(File const& rhs) {
  if (this != &rhs) {
    absolute_path_ = rhs.absolute_path_;
    is_loaded_ = rhs.is_loaded_;
    stat_ = rhs.stat_;
  }
  return *this;
}

// autoindexでsortを使用したいので作成
// ディレクトリが上にきてほしい。
bool operator<(const File& lhs, const File& rhs) {
  if (lhs.IsDir() != rhs.IsDir()) {
    return lhs.IsDir() ? true : false;
  }
  return lhs.GetAbsolutePath() < rhs.GetAbsolutePath();
}

bool operator>(const File& lhs, const File& rhs) {
  return rhs < lhs;
}

bool operator<=(const File& lhs, const File& rhs) {
  return !(lhs > rhs);
}

bool operator>=(const File& lhs, const File& rhs) {
  return !(lhs < rhs);
}

std::string File::GetAbsolutePath() const {
  return absolute_path_;
}

bool File::Load() {
  // statが失敗するケースで file classを使うパターンがあるか？
  // パーミッションがない時、stat失敗するか調べる
  if (stat(absolute_path_.c_str(), &stat_)) {
    std::cerr << "File class stat err" << std::endl;
    return false;
  }
  is_loaded_ = true;
  return true;
}

// TODO : 最後に / が入ってるとき正常に機能しない？
// TODO : テストの作成
std::string File::GetFileName() const {
  std::string::size_type pos;

  if (absolute_path_ == "/") {
    return "/";
  }

  pos = absolute_path_.rfind("/");
  if (pos == std::string::npos) {
    return absolute_path_;
  }

  std::string file_name =
      absolute_path_.substr(pos + 1, absolute_path_.length() - pos);
  if (IsDir()) {
    return file_name + "/";
  } else {
    return file_name;
  }
}

bool File::IsDir() const {
  assert(is_loaded_);
  return S_ISDIR(stat_.st_mode);
}

std::string File::GetFileSizeStr() const {
  std::stringstream ss;

  assert(is_loaded_);

  if (IsDir()) {
    return "-";
  }
  ss << stat_.st_size;
  return ss.str();
}

std::string File::GetDateStr(const std::string fmt) const {
  char buf[256];
  struct tm* tm;

  assert(is_loaded_);

  tm = gmtime(&stat_.st_atime);
  strftime(buf, 256, fmt.c_str(), tm);
  return std::string(buf);
}

}  // namespace utils
