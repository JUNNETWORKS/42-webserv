#include "http/chunk_writer.hpp"

#include <unistd.h>

#include "http/http_constants.hpp"
#include "utils/string.hpp"

namespace http {

ChunkWriter::ChunkWriter()
    : current_chunk_size_(0),
      written_size_to_chunk_(0),
      is_written_last_chunk_(false) {}

ChunkWriter::~ChunkWriter() {}

void ChunkWriter::AppendDataToBuffer(utils::ByteVector &buf) {
  buffer_.insert(buffer_.end(), buf.begin(), buf.end());
}

Result<void> ChunkWriter::Write(const int sock_fd) {
  if (written_size_to_chunk_ == current_chunk_size_) {
    written_size_to_chunk_ = 0;
    current_chunk_size_ = CalculateChunkSize();
    if (current_chunk_size_ == 0) {
      // 書き込むデータがまだない
      return Result<void>();
    }

    // chunk header を追加
    std::stringstream ss;
    ss << std::hex << current_chunk_size_;
    std::string chunk_header = ss.str();
    chunk_header += kCrlf;
    buffer_.insert(buffer_.begin(), chunk_header.begin(), chunk_header.end());
    current_chunk_size_ += chunk_header.size();

    // chunk 末尾のCRLFを追加
    buffer_.insert(buffer_.begin() + current_chunk_size_, kCrlf.begin(),
                   kCrlf.end());
    current_chunk_size_ += kCrlf.size();

    return Result<void>();
  }

  ssize_t write_res = write(sock_fd, buffer_.data(),
                            current_chunk_size_ - written_size_to_chunk_);
  if (write_res < 0) {
    return Error();
  }
  buffer_.EraseHead(write_res);
  written_size_to_chunk_ += write_res;
  return Result<void>();
}

Result<void> ChunkWriter::WriteEndOfChunk(const int sock_fd) {
  is_written_last_chunk_ = true;
  std::string last_chunk = "0";
  last_chunk += kCrlf + kCrlf;
  if (write(sock_fd, last_chunk.c_str(), last_chunk.size()) < 0) {
    return Error();
  }
  return Result<void>();
}

bool ChunkWriter::IsBufferEmpty() {
  return buffer_.empty();
}

bool ChunkWriter::IsWrittenLastChunk() {
  return is_written_last_chunk_;
}

unsigned long ChunkWriter::CalculateChunkSize() {
  if (buffer_.size() >= kMaxChunkSize) {
    return kMaxChunkSize;
  } else {
    return buffer_.size();
  }
}

}  // namespace http
