#include "utils/io.hpp"

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "utils/File.hpp"

namespace utils {

// TODO :
// 使い方が合っているか確認、ファイルが存在していてもパーミッションがないとfalseが帰る？
bool IsFileExist(const std::string& file_path) {
  std::ifstream ifs(file_path.c_str());
  return ifs.is_open();
}

bool IsDir(const std::string& file_path) {
  struct stat sb;

  stat(file_path.c_str(), &sb);
  return S_ISDIR(sb.st_mode);
}

Result<std::vector<utils::File> > GetFileList(const std::string& target_dir) {
  std::vector<utils::File> vec;
  struct dirent* dent;
  DIR* dir = opendir(target_dir.c_str());

  if (dir == NULL) {
    return Error();
  }

  while ((dent = readdir(dir))) {  // TODO : readdir err ck
    File f(target_dir + "/" + dent->d_name);
    if (f.GetFileType() == utils::FileType::kNotExist) {
      continue;
    }
    vec.push_back(f);
  }
  closedir(dir);
  return vec;
}

bool PutStrFdBase(const char* str, size_t len, int fd) {
  ssize_t write_byte;

  while (len > 0) {
    if (len > INT_MAX) {
      write_byte = write(fd, str, INT_MAX);
    } else {
      write_byte = write(fd, str, len);
    }
    if (write_byte < 0) {  // TODO : errno EINTR
      return false;
    }
    len -= write_byte;
    str += write_byte;
  }
  return true;
}

bool PutStrFd(const std::string str, int fd) {
  return PutStrFdBase(str.c_str(), str.length(), fd);
}

}  // namespace utils
