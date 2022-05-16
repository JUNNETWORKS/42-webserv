#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/utils.hpp"

namespace utils {

void errExit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}

}  // namespace utils
