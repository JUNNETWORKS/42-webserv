#ifndef UTILS_IO_HPP
#define UTILS_IO_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "result/result.hpp"
#include "utils/File.hpp"

namespace utils {

using namespace result;

bool IsFileExist(const std::string& path);

bool IsDir(const std::string& path);

bool IsRegularFile(const std::string& file_path);

unsigned long GetFileSize(const std::string& file_path);

bool IsReadableFile(const std::string& file_path);

bool IsExecutableFile(const std::string& file_path);

Result<std::vector<utils::File> > GetFileList(const std::string& target_dir);

bool PutStrFd(const std::string str, int fd);

}  // namespace utils

#endif
