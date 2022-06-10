#include "result/result.hpp"

namespace result {

Error::Error(bool is_err) : is_err_(is_err) {}

Error::Error(const std::string &err_msg) : is_err_(true), err_msg_(err_msg) {}

Error::Error(const char *err_msg) : is_err_(true), err_msg_(err_msg) {}

Error::Error(const Error &other) {
  *this = other;
}

const Error &Error::operator=(const Error &rhs) {
  if (this != &rhs) {
    is_err_ = rhs.is_err_;
    err_msg_ = rhs.err_msg_;
  }
  return *this;
}

Error::~Error() {}

bool Error::IsErr() {
  return is_err_;
}

std::string Error::GetMessage() {
  return err_msg_;
}

}  // namespace result
