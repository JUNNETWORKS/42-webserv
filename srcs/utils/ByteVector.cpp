#include "ByteVector.hpp"

#include <stdio.h>

namespace utils {

ByteVector::ByteVector() {
  reserve(kReserveSize_);
}
ByteVector::ByteVector(const std::string& s) {
  AppendDataToBuffer(reinterpret_cast<const unsigned char*>(s.data()),
                     s.size());
}
ByteVector::ByteVector(ByteVector const& src) : std::vector<Byte>(src) {
  *this = src;
}
ByteVector::~ByteVector() {}

ByteVector& ByteVector::operator=(ByteVector const& rhs) {
  std::vector<Byte>::operator=(rhs);
  return *this;
}

void ByteVector::EraseHead(size_t size) {
  erase(begin(), begin() + size);
}

bool ByteVector::CompareHead(const std::string& str) {
  if (size() < str.size())
    return false;
  return std::strncmp(GetReinterpretedData(), str.c_str(), str.size()) == 0;
}

ByteVector::iterator ByteVector::FindString(const std::string& str) {
  push_back('\0');

  const char* start = GetReinterpretedData();
  const char* char_pos = std::strstr(start, str.c_str());
  bool find_res = char_pos != NULL;
  size_t pos;

  if (find_res)
    pos = char_pos - start;

  pop_back();

  return find_res ? iterator(&((*this)[pos])) : end();
}

std::string ByteVector::CutSubstrBeforePos(ByteVector::iterator pos) {
  std::string res = std::string(begin(), pos);
  erase(begin(), pos);
  return res;
}

void ByteVector::AppendDataToBuffer(const Byte* buf, size_t size) {
  insert(end(), buf, buf + size);
  printf("current buf len: %lu\n", this->size());
}

const char* ByteVector::GetReinterpretedData() {
  return reinterpret_cast<const char*>(data());
}

}  // namespace utils
