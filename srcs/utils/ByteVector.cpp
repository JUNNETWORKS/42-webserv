#include "ByteVector.hpp"

namespace utils {
std::string ExtractStrFromByteVector(ByteVector &vec, const char *pos) {
  const char *buffer_ptr = reinterpret_cast<const char *>(vec.data());
  size_t size = pos - buffer_ptr;
  std::string res(buffer_ptr, size);
  vec.erase(vec.begin(), vec.begin() + size);

  return res;
};

const char *FindStrFromByteVector(ByteVector &vec, const std::string &str) {
  vec.push_back('\0');
  const char *res =
      std::strstr(reinterpret_cast<const char *>(vec.data()), str.c_str());
  vec.pop_back();
  return res;
};

bool CompareByteVectorHead(ByteVector &vec, const std::string &str) {
  if (vec.size() < str.size())
    return false;
  return std::strncmp(reinterpret_cast<const char *>(vec.data()), str.c_str(),
                      str.size()) == 0;
};

void EraseByteVectorHead(ByteVector &vec, size_t size) {
  vec.erase(vec.begin(), vec.begin() + size);
}

}  // namespace utils