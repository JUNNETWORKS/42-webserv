#include "utils.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void usageErr(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, "usage\n", args);
  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}

void errExit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}

void fatal(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vfprintf(stderr, "failed ", args);
  vfprintf(stderr, fmt, args);

  va_end(args);
  exit(1);
}
