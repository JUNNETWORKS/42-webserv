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

Result<bool> IsDir(const std::string& file_path) {
  struct stat sb;

  if (stat(file_path.c_str(), &sb) < 0) {
    return Error();
  }
  return S_ISDIR(sb.st_mode);
}

Result<bool> IsRegularFile(const std::string& file_path) {
  struct stat sb;

  if (stat(file_path.c_str(), &sb) < 0) {
    return Error();
  }
  return S_ISREG(sb.st_mode);
}

Result<unsigned long> GetFileSize(const std::string& file_path) {
  struct stat sb;

  if (stat(file_path.c_str(), &sb) < 0) {
    return Error();
  }
  return sb.st_size;
}

bool IsFileExist(const std::string& file_path) {
  return access(file_path.c_str(), F_OK) == 0;
}

bool IsReadableFile(const std::string& file_path) {
  return access(file_path.c_str(), R_OK) == 0;
}

Result<bool> IsExecutableFile(const std::string& file_path) {
  bool is_executable = access(file_path.c_str(), X_OK) == 0;

  if (is_executable == false) {
    return false;
  }
  Result<bool> is_regular_file_res = utils::IsRegularFile(file_path);
  if (is_regular_file_res.IsErr()) {
    return Error();
  }
  if (is_regular_file_res.Ok() == false) {
    return false;
  }
  return true;
}

Result<std::vector<utils::File> > GetFileList(const std::string& target_dir) {
  std::vector<utils::File> vec;
  struct dirent* dent;
  DIR* dir = opendir(target_dir.c_str());

  if (dir == NULL) {
    return Error();
  }

  while ((dent = readdir(dir))) {
    File f(target_dir + "/" + dent->d_name);
    if (f.GetFileType() == File::kNotExist) {
      continue;
    }
    vec.push_back(f);
  }
  closedir(dir);
  return vec;
}

}  // namespace utils
