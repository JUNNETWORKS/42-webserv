#ifndef FILE_HPP_
#define FILE_HPP_

#include <sys/stat.h>

#include <iostream>

namespace utils {

class File {
 public:
  File(const std::string& absolute_path);
  File(const File& rhs);
  ~File();

  File& operator=(const File& rhs);
  bool operator<(const File& rhs);

  std::string GetFileName() const;

  bool IsDir() const;
  std::string GetFileSizeStr() const;
  std::string GetDateStr(const std::string fmt) const;

 private:
  std::string absolute_path_;
  struct stat stat_;
};

}  // namespace utils

#endif
