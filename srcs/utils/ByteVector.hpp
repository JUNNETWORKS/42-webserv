#ifndef BYTEVECTOR_HPP_
#define BYTEVECTOR_HPP_

#include <cstring>
#include <string>
#include <vector>

#include "http/http_constants.hpp"

namespace utils {
typedef unsigned char Byte;
typedef std::vector<Byte> ByteVector;

std::string ExtractStrFromByteVector(ByteVector &vec, const char *pos);
const char *FindStrFromByteVector(ByteVector &vec, const std::string &str);
bool CompareByteVectorHead(ByteVector &vec, const std::string &str);
void EraseByteVectorHead(ByteVector &vec, size_t size);

}  // namespace utils

#endif
