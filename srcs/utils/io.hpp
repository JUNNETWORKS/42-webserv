#ifndef UTILS_IO_HPP
#define UTILS_IO_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace utils {

bool IsFileExist(const std::string& path);

bool IsDir(const std::string& path);

bool GetFileList(const std::string& path, std::vector<std::string>& vec);

bool PutStrFd(const std::string str, int fd);

}  // namespace utils

#endif
