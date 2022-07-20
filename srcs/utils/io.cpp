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

bool IsDir(const std::string& file_path) {
  struct stat sb;

  stat(file_path.c_str(), &sb);
  return S_ISDIR(sb.st_mode);
}

bool IsRegularFile(const std::string& file_path) {
  struct stat sb;

  stat(file_path.c_str(), &sb);
  return S_ISREG(sb.st_mode);
}

// TODO: Resultで返したほうがいいかも?
unsigned long GetFileSize(const std::string& file_path) {
  struct stat sb;

  stat(file_path.c_str(), &sb);
  return sb.st_size;
}

bool IsFileExist(const std::string& file_path) {
  return access(file_path.c_str(), F_OK) == 0;
}

bool IsReadableFile(const std::string& file_path) {
  return access(file_path.c_str(), R_OK) == 0;
}

bool IsExecutableFile(const std::string& file_path) {
  return access(file_path.c_str(), X_OK) == 0;
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
    if (f.GetFileType() == File::kNotExist) {
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
