#include "utils/error.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace utils {

void ErrExit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}

}  // namespace utils
