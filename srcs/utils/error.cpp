#include "utils/error.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace utils {

void ErrExit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}

}  // namespace utils
