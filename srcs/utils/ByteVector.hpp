#ifndef BYTEVECTOR_HPP_
#define BYTEVECTOR_HPP_

#include <cstring>
#include <string>
#include <vector>

#include "http/http_constants.hpp"

namespace utils {
typedef unsigned char Byte;

class ByteVector {
 public:
  typedef std::vector<Byte> container_type;
  typedef container_type::iterator iterator;

  ByteVector();
  ByteVector(ByteVector const& src);
  ~ByteVector();

  ByteVector& operator=(ByteVector const& rhs);

  iterator begin();
  iterator end();

  void EraseHead(size_t size);
  bool CompareHead(const std::string& str);
  iterator FindString(const std::string& str);
  std::string ExtractBeforePos(iterator pos);

  void AppendDataToBuffer(Byte* buf, size_t size);

 private:
  static const container_type::size_type kReserveSize_ = 2 * 1024;  // 2KB

  const char* GetReinterpretedData();

  container_type vec_;
};

}  // namespace utils

#endif
