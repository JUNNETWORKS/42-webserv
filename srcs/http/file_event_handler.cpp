#include "http/file_event_handler.hpp"

#include <unistd.h>

#include <algorithm>

#include "http/http_response.hpp"

namespace http {

void HandleFileEvent(FdEvent *fde, unsigned int events, void *data,
                     Epoll *epoll) {
  FileBuffer *file_buffer = reinterpret_cast<FileBuffer *>(data);
  file_buffer->events = events;

  if (file_buffer->is_eof) {
    return;
  }
  if (file_buffer->is_unregistered) {
    // HttpResponse側で必要なくなった
    delete fde;
    delete file_buffer;
    return;
  }

  if (events & kFdeRead) {
    if (file_buffer->buffer.size() >= FileBuffer::kMaxBufferSize) {
      return;
    }
    utils::Byte buf[FileBuffer::kBytesPerRead];
    // kBytesPerRead + 0 のようにしているのは､
    // static const メンバ定数は参照として使えないので､
    // 演算を行うことで右辺値に変換している｡
    int read_res =
        read(fde->fd, buf,
             std::min(FileBuffer::kBytesPerRead + 0,
                      FileBuffer::kMaxBufferSize - file_buffer->buffer.size()));

    if (read_res < 0) {
      // Error
      file_buffer->events = kFdeError;
      epoll->Unregister(fde);
      file_buffer->is_unregistered = true;
      return;
    } else if (read_res == 0) {
      // EOF
      file_buffer->is_eof = true;
      epoll->Unregister(fde);
      file_buffer->is_unregistered = true;
      return;
    }
    file_buffer->buffer.AppendDataToBuffer(buf, read_res);
  }
  if (events & kFdeError) {
    file_buffer->events = kFdeError;
    epoll->Unregister(fde);
    file_buffer->is_unregistered = true;
    return;
  }
}

}  // namespace http
