#ifndef FILE_HPP_
#define FILE_HPP_

#include <sys/stat.h>

#include <iostream>

namespace utils {

class File {
 public:
  // enum の順番が sort の順番になる
  enum FileType { kNotExist, kDir, kFile };

  File(const std::string &absolute_path);
  File(const File &rhs);
  ~File();

  File &operator=(const File &rhs);

  std::string GetAbsolutePath() const;
  std::string GetFileName() const;
  File::FileType GetFileType() const;

  void SetFileType();

  bool IsDir() const;

  // FileType が kNotExist の場合呼び出し NG
  std::string GetFileSizeStr() const;
  std::string GetDateStr(const std::string fmt) const;

 private:
  std::string absolute_path_;
  struct stat stat_;
  FileType file_type_;
};

bool operator>(const File &lhs, const File &rhs);
bool operator<(const File &lhs, const File &rhs);
bool operator>=(const File &lhs, const File &rhs);
bool operator<=(const File &lhs, const File &rhs);

}  // namespace utils

#endif
