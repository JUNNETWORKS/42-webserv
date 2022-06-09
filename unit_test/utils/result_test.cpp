#include "utils/result.hpp"

#include <gtest/gtest.h>

namespace utils {
namespace {

class NoConstructorClass {
 private:
  int n_;

 public:
  NoConstructorClass(int n) : n_(n) {}

  int GetN() {
    return n_;
  }

 private:
  NoConstructorClass();
};

Result<int> ReturnInt() {
  return 10;
}
TEST(ResultTest, ReturnInt) {
  Result<int> result = ReturnInt();
  EXPECT_TRUE(result.IsOk());
  EXPECT_EQ(result.Ok(), 10);
  EXPECT_FALSE(result.IsErr());
}

Result<int> ReturnIntError() {
  return Result<int>(10, Error("Error ReturnIntError"));
}
TEST(ResultTest, ReturnIntError) {
  Result<int> result = ReturnIntError();
  EXPECT_FALSE(result.IsOk());
  EXPECT_TRUE(result.IsErr());
  EXPECT_EQ(result.Err().Print(), "Error ReturnIntError");
}

Result<void> ReturnVoid() {
  // void の時は明示的にインスタンスを作成してreturnする必要がある.
  return Result<void>();
}
TEST(ResultTest, ReturnVoid) {
  Result<void> result = ReturnVoid();
  EXPECT_TRUE(result.IsOk());
  EXPECT_FALSE(result.IsErr());
}

Result<void> ReturnVoidError() {
  return Error("Error ReturnVoidError");
}
TEST(ResultTest, ReturnVoidError) {
  Result<void> result = ReturnVoidError();
  EXPECT_FALSE(result.IsOk());
  EXPECT_TRUE(result.IsErr());
  EXPECT_EQ(result.Err().Print(), "Error ReturnVoidError");
}

Result<std::vector<int> > ReturnVector() {
  std::vector<int> vec;
  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }
  return vec;
}
TEST(ResultTest, ReturnVector) {
  Result<std::vector<int> > result = ReturnVector();
  EXPECT_TRUE(result.IsOk());
  EXPECT_FALSE(result.IsErr());
  const std::vector<int>& vec = result.Ok();
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec[i], i);
  }
}

Result<NoConstructorClass> ReturnNoConstructorObject() {
  return NoConstructorClass(10);
}
TEST(ResultTest, ReturnNoConstructorObject) {
  Result<NoConstructorClass> result = ReturnNoConstructorObject();
  EXPECT_TRUE(result.IsOk());
  EXPECT_EQ(result.Ok().GetN(), 10);
  EXPECT_FALSE(result.IsErr());
}

}  // namespace
}  // namespace utils
