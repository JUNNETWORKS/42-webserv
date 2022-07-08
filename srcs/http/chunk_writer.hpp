#ifndef HTTP_CHUNK_WRITER_HPP
#define HTTP_CHUNK_WRITER_HPP

#include "result/result.hpp"
#include "utils/ByteVector.hpp"

namespace http {

using namespace result;

class ChunkWriter {
 private:
  static const unsigned long kMaxChunkSize = 1024;  // 1KB

  utils::ByteVector buffer_;

  // 現在書き込んでいるチャンクは何バイト書き込むか
  unsigned long current_chunk_size_;

  // 現在書き込んでいるチャンクの内何バイト書き込み完了したか
  unsigned long written_size_to_chunk_;

 public:
  ChunkWriter();
  ~ChunkWriter();

  void AppendDataToBuffer(utils::ByteVector &buf);
  Result<void> Write(int sock_fd);

  // データの追加が遅れている場合にチャンク終了を書き込まないように
  // last-chunk は別の関数で処理する｡
  Result<void> WriteEndOfChunk(int sock_fd);

 private:
  Result<void> WriteChunkHeader(int sock_fd);
  unsigned long CalculateChunkSize();
};

}  // namespace http

#endif
