#include "ByteVector.hpp"

#include <stdio.h>

namespace utils {

ByteVector::ByteVector() : vec_() {
  vec_.reserve(kReserveSize_);
}
ByteVector::ByteVector(ByteVector const& src) : vec_() {
  *this = src;
}
ByteVector::~ByteVector() {}

ByteVector& ByteVector::operator=(ByteVector const& rhs) {
  if (this != &rhs) {
    this->vec_ = rhs.vec_;
  }
  return *this;
}

ByteVector::iterator ByteVector::begin() {
  return vec_.begin();
}

ByteVector::iterator ByteVector::end() {
  return vec_.end();
}

void ByteVector::EraseHead(size_t size) {
  vec_.erase(vec_.begin(), vec_.begin() + size);
}

bool ByteVector::CompareHead(const std::string& str) {
  if (vec_.size() < str.size())
    return false;
  return std::strncmp(GetReinterpretedData(), str.c_str(), str.size()) == 0;
}

ByteVector::iterator ByteVector::FindString(const std::string& str) {
  vec_.push_back('\0');

  const char* start = GetReinterpretedData();
  const char* char_pos = std::strstr(start, str.c_str());
  bool find_res = char_pos != NULL;
  size_t pos;

  if (find_res)
    pos = char_pos - start;

  vec_.pop_back();

  return find_res ? iterator(&(vec_[pos])) : end();
}

std::string ByteVector::ExtractBeforePos(ByteVector::iterator pos) {
  std::string res = std::string(begin(), pos);
  vec_.erase(begin(), pos);
  return res;
}

void ByteVector::AppendDataToBuffer(Byte* buf, size_t size) {
  vec_.insert(vec_.end(), buf, buf + size);
  printf("current buf len: %lu\n", vec_.size());
}

const char* ByteVector::GetReinterpretedData() {
  return reinterpret_cast<const char*>(vec_.data());
}

}  // namespace utils