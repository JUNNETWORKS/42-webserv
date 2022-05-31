#ifndef UTILS_ERROR_HPP
#define UTILS_ERROR_HPP

namespace utils {

// stderr に出力し､exit(1)する｡
void ErrExit(const char *fmt, ...);

}  // namespace utils

#endif
