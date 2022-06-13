#include "time.hpp"

#include <stdlib.h>
#include <sys/time.h>

namespace utils {

long GetCurrentTimeMs() {
  timeval tv;

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

}  // namespace utils
