#include "http/chunk_writer.hpp"

#include <unistd.h>

#include "http/http_constants.hpp"
#include "utils/string.hpp"

namespace http {

ChunkWriter::ChunkWriter()
    : current_chunk_size_(0), written_size_to_chunk_(0) {}

ChunkWriter::~ChunkWriter() {}

void ChunkWriter::AppendDataToBuffer(utils::ByteVector &buf) {
  buffer_.insert(buffer_.end(), buf.begin(), buf.end());
}

Result<void> ChunkWriter::Write(int sock_fd) {
  if (written_size_to_chunk_ == current_chunk_size_) {
    written_size_to_chunk_ = 0;
    current_chunk_size_ = 0;
    return WriteChunkHeader(sock_fd);
  }
  ssize_t write_res = write(sock_fd, buffer_.data(),
                            current_chunk_size_ - written_size_to_chunk_);
  if (write_res < 0) {
    return Error();
  }
  buffer_.EraseHead(write_res);
  written_size_to_chunk_ += write_res;
  if (written_size_to_chunk_ == current_chunk_size_) {
    if (write(sock_fd, kCrlf.c_str(), kCrlf.size()) < 0) {
      return Error();
    }
  }
  return Result<void>();
}

Result<void> ChunkWriter::WriteEndOfChunk(int sock_fd) {
  std::string last_chunk = "0";
  last_chunk += kCrlf;
  if (write(sock_fd, last_chunk.c_str(), last_chunk.size()) < 0) {
    return Error();
  }
  return Result<void>();
}

Result<void> ChunkWriter::WriteChunkHeader(int sock_fd) {
  current_chunk_size_ = CalculateChunkSize();
  if (current_chunk_size_ == 0) {
  }

  std::string chunk_header = utils::ConvertToStr(current_chunk_size_);
  chunk_header += kCrlf;
  if (write(sock_fd, chunk_header.c_str(), chunk_header.size()) < 0) {
    return Error();
  }
  return Result<void>();
}

unsigned long ChunkWriter::CalculateChunkSize() {
  if (buffer_.size() >= kMaxChunkSize) {
    return kMaxChunkSize;
  } else {
    return buffer_.size();
  }
}

}  // namespace http
