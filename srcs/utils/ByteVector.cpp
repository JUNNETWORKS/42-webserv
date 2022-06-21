#include "ByteVector.hpp"

#include <stdio.h>

namespace utils {

ByteVector::ByteVector() {
  reserve(kReserveSize_);
}
ByteVector::ByteVector(ByteVector::const_iterator start,
                       ByteVector::const_iterator end)
    : std::vector<Byte>(start, end) {}

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

bool ByteVector::CompareHead(const std::string& str) const {
  if (size() < str.size())
    return false;
  return std::strncmp(GetReinterpretedData(), str.c_str(), str.size()) == 0;
}

Result<size_t> ByteVector::FindString(const std::string& str) const {
  if (str.size() > size()) {
    return Error();
  }
  for (size_t i = 0; i < (size() - str.size() + 1); i++) {
    for (size_t j = 0; j < str.size(); j++) {
      if (str[j] != this->at(i + j))
        break;
      if (j + 1 == str.size())
        return i;
    }
  }
  return Error();
}

std::string ByteVector::CutSubstrBeforePos(size_t pos) {
  std::string res = std::string(begin(), begin() + pos);
  erase(begin(), begin() + pos);
  return res;
}

std::string ByteVector::SubstrBeforePos(size_t pos) const {
  std::string res = std::string(begin(), begin() + pos);
  return res;
}

void ByteVector::AppendDataToBuffer(const Byte* buf, size_t size) {
  insert(end(), buf, buf + size);
  printf("current buf len: %lu\n", this->size());
}

const char* ByteVector::GetReinterpretedData() const {
  return reinterpret_cast<const char*>(data());
}

}  // namespace utils
