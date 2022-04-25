#ifndef RDWRN_HPP_
#define RDWRN_HPP_

#include <stdlib.h>

// ソケットに対する read, write はソケットバッファやシグナルによって
// 全データの読み書きができない可能性があるので､
// すべてのデータを確実に読み書きできるように内部でループを回す関数

ssize_t readn(int fd, void *buffer, size_t n);
ssize_t writen(int fd, void *buffer, size_t n);

#endif
