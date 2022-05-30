#include "utils/io.hpp"

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace utils {

// TODO
bool isFileExist(const std::string& path) {
  std::ifstream ifs(path.c_str());
  return ifs.is_open();
}

bool isDir(const std::string& path) {
  struct stat sb;

  stat(path.c_str(), &sb);
  return S_ISDIR(sb.st_mode);
}

bool getFileList(const std::string& path, std::vector<std::string>& vec) {
  struct dirent* dent;
  DIR* dir = opendir(path.c_str());

  if (dir == NULL) {
    return false;
  }

  while ((dent = readdir(dir))) {  // TODO : readdir err ck
    vec.push_back(std::string(dent->d_name));
  }
  closedir(dir);
  return true;
}

bool ft_putstr_fd_base(const char* str, size_t len, int fd) {
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

bool ft_putstr_fd(const std::string str, int fd) {
  return ft_putstr_fd_base(str.c_str(), str.length(), fd);
}

}  // namespace utils
