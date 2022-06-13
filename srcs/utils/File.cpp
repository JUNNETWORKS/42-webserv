#include "File.hpp"

#include <sys/stat.h>

#include <sstream>

namespace utils {

File::File(const std::string& absolute_path) : absolute_path_(absolute_path) {
  // TODO : コンストラクタでやらないようにするなど検討。
  // statが失敗するケースで file classを使うパターンがあるか？
  // パーミッションがない時、stat失敗するか調べる
  if (stat(absolute_path.c_str(), &stat_)) {
    std::cerr << "File class stat err" << std::endl;
  }
}

File::File(const File& rhs) {
  *this = rhs;
}

File::~File() {}

File& File::operator=(File const& rhs) {
  if (this != &rhs) {
    absolute_path_ = rhs.absolute_path_;
    stat_ = rhs.stat_;
  }
  return *this;
}

// autoindexでsortを使用したいので作成
// ディレクトリが上にきてほしい。
bool File::operator<(File const& rhs) {
  if (IsDir() != rhs.IsDir()) {
    return IsDir() ? true : false;
  }
  return absolute_path_ < rhs.absolute_path_;
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
  return S_ISDIR(stat_.st_mode);
}

std::string File::GetFileSizeStr() const {
  std::stringstream ss;

  if (IsDir()) {
    return "-";
  }
  ss << stat_.st_size;
  return ss.str();
}

std::string File::GetDateStr(const std::string fmt) const {
  char buf[256];
  struct tm* tm;

  tm = gmtime(&stat_.st_atime);
  strftime(buf, 256, fmt.c_str(), tm);
  return std::string(buf);
}

}  // namespace utils
