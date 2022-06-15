#ifndef FILE_HPP_
#define FILE_HPP_

#include <sys/stat.h>

#include <iostream>

namespace utils {

// enum の順番が sort の順番になる
enum FileType { kNotExist, kDir, kFile };

class File {
 public:
  File(const std::string &absolute_path);
  File(const File &rhs);
  ~File();

  File &operator=(const File &rhs);

  std::string GetAbsolutePath() const;
  std::string GetFileName() const;
  FileType GetFileType() const;

  void SetFileType();

  bool IsDir() const;
  std::string GetFileSizeStr() const;
  std::string GetDateStr(const std::string fmt) const;

 private:
  std::string absolute_path_;
  bool is_loaded_;
  struct stat stat_;
  FileType file_type_;
};

bool operator>(const File &lhs, const File &rhs);
bool operator<(const File &lhs, const File &rhs);
bool operator>=(const File &lhs, const File &rhs);
bool operator<=(const File &lhs, const File &rhs);

}  // namespace utils

#endif
