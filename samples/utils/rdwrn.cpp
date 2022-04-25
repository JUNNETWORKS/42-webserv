#include "rdwrn.hpp"

#include <errno.h>
#include <unistd.h>

ssize_t readn(int fd, void *buffer, size_t n) {
  ssize_t numRead; /* # of bytes fetched by last read() */
  size_t totRead;  /* Total # of bytes read so far */
  char *buf;

  buf = (char *)buffer; /* No pointer arthmetic on "void *" */
  for (totRead = 0; totRead < n;) {
    numRead = read(fd, buf, n - totRead);

    if (numRead == 0) /* EOF */
      return totRead; /* May be 0 if this is first read */
    if (numRead == -1) {
      if (errno == EINTR)
        continue; /* Interrupted --> restart read() */
      else
        return -1; /* Some other error */
    }
    totRead += numRead;
    buf += numRead;
  }
  return totRead;
}

ssize_t writen(int fd, void *buffer, size_t n) {
  ssize_t numWritten; /* # of bytes written by last write() */
  size_t totWritten;  /* Total # of bytes written so far */
  const char *buf;

  buf = (const char *)buffer; /* No pointer arthmetic on "void *" */
  for (totWritten = 0; totWritten < n;) {
    numWritten = write(fd, buf, n - totWritten);

    if (numWritten < 0) {
      if (numWritten == -1 && errno == EINTR)
        continue;
      else
        return -1;
    }
    totWritten += numWritten;
    buf += numWritten;
  }
  return numWritten;
}
