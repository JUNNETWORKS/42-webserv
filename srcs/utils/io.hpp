#ifndef UTILS_IO_HPP
#define UTILS_IO_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace utils {

bool isFileExist(const std::string& path);

bool isDir(const std::string& path);

bool getFileList(const std::string& path, std::vector<std::string>& vec);

bool ft_putstr_fd(const std::string str, int fd);

}  // namespace utils

#endif
