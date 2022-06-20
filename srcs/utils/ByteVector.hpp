#ifndef BYTEVECTOR_HPP_
#define BYTEVECTOR_HPP_

#include <cstring>
#include <string>
#include <vector>

#include "result/result.hpp"

namespace utils {

using namespace result;

typedef unsigned char Byte;

class ByteVector : public std::vector<Byte> {
 public:
  ByteVector();
  ByteVector(ByteVector::const_iterator start, ByteVector::const_iterator end);
  ByteVector(ByteVector const& src);
  ByteVector(const std::string& s);
  ~ByteVector();

  ByteVector& operator=(ByteVector const& rhs);

  void EraseHead(size_t size);
  bool CompareHead(const std::string& str) const;
  Result<unsigned long> FindString(const std::string& str) const;
  std::string CutSubstrBeforePos(size_t pos);
  std::string SubstrBeforePos(size_t pos) const;
  void AppendDataToBuffer(const Byte* buf, size_t size);

 private:
  static const size_type kReserveSize_ = 2 * 1024;  // 2KB

  const char* GetReinterpretedData() const;
};

}  // namespace utils

#endif
