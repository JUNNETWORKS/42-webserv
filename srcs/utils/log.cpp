#include "log.hpp"

namespace utils {
void PrintLog(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

void PrintDebugLog(const char *fmt, ...) {
#ifndef DEBUG
  return;
#endif
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

}  // namespace utils
