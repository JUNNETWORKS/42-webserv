#include "read_line.hpp"

#include <errno.h>
#include <unistd.h>

ssize_t readLine(int fd, void *buffer, size_t n) {
  ssize_t numRead; /* # of bytes fetched by last read() */
  size_t totRead;  /* Total bytes read so far */
  char *buf;
  char ch;

  if (n <= 0 || buffer == NULL) {
    errno = EINVAL;
    return -1;
  }

  buf = (char *)buffer;
  totRead = 0;
  while (true) {
    numRead = read(fd, &ch, 1);

    if (numRead == -1) {
      if (errno == EINTR) /* Interrupted --> restart read() */
        continue;
      else
        return -1;
    } else if (numRead == 0) { /* EOF */
      if (totRead == 0)
        return 0;
      else /* Some bytes read; add '\0' */
        break;
    } else {
      if (totRead < n - 1) {
        totRead++;
        *buf++ = ch;
      }
      if (ch == '\n')
        break;
    }
  }
  *buf = '\0';
  return totRead;
}
