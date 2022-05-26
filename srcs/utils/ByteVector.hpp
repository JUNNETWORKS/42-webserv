#ifndef BYTEVECTOR_HPP_
#define BYTEVECTOR_HPP_

#include <cstring>
#include <string>
#include <vector>

#include "http/http_constants.hpp"

namespace utils {
typedef unsigned char Byte;

class ByteVector : public std::vector<Byte> {
 public:
  ByteVector();
  ByteVector(ByteVector const& src);
  ~ByteVector();

  ByteVector& operator=(ByteVector const& rhs);

  void EraseHead(size_t size);
  bool CompareHead(const std::string& str);
  iterator FindString(const std::string& str);
  std::string ExtractBeforePos(iterator pos);

  void AppendDataToBuffer(Byte* buf, size_t size);

 private:
  static const size_type kReserveSize_ = 2 * 1024;  // 2KB

  const char* GetReinterpretedData();
};

}  // namespace utils

#endif
