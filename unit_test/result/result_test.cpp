#include "result/result.hpp"

#include <gtest/gtest.h>

namespace result {

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
  EXPECT_EQ(result.Err().GetMessage(), "Error ReturnIntError");
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
  EXPECT_EQ(result.Err().GetMessage(), "Error ReturnVoidError");
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

Result<NoConstructorClass> ReturnNoConstructorObjectAndError() {
  // デフォルトコンストラクタが利用できない場合は明示的にResultインスタンスを作成する必要がある｡
  return Result<NoConstructorClass>(
      NoConstructorClass(0), Error("Error ReturnNoConstructorObjectAndError"));
}
TEST(ResultTest, ReturnNoConstructorObjectAndError) {
  Result<NoConstructorClass> result = ReturnNoConstructorObjectAndError();
  EXPECT_FALSE(result.IsOk());
  EXPECT_TRUE(result.IsErr());
  EXPECT_EQ(result.Err().GetMessage(),
            "Error ReturnNoConstructorObjectAndError");
}

TEST(ResultTest, ErrorDefaultConstructor) {
  Result<void> result = Error();
  EXPECT_FALSE(result.IsOk());
  EXPECT_TRUE(result.IsErr());
}

class ClassWithReferenceVar {
 private:
  int n_;

 public:
  ClassWithReferenceVar(int n) : n_(n) {}

  Result<int&> GetN() {
    return n_;
  }

  Result<int&> GetError() {
    return Error();
  }

 private:
  ClassWithReferenceVar();
};

TEST(ResultTest, ResultOfReference) {
  ClassWithReferenceVar obj = ClassWithReferenceVar(10);

  Result<int&> result_ok = obj.GetN();
  EXPECT_TRUE(result_ok.IsOk());
  EXPECT_EQ(result_ok.Ok(), 10);

  Result<int&> result_err = obj.GetError();
  EXPECT_TRUE(result_err.IsErr());
}

}  // namespace result
