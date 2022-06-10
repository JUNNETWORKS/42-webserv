#ifndef UTILS_RESULT_HPP_
#define UTILS_RESULT_HPP_

#include <cassert>
#include <string>

namespace result {

// Result でエラーを表すためのクラス
class Error {
 private:
  bool is_err_;
  std::string err_msg_;

 public:
  explicit Error(bool is_err = true);

  // エラーメッセージを設定する｡
  // is_err は true に設定される｡
  Error(const std::string &err_msg);

  // エラーメッセージを設定する｡
  // is_err は true に設定される｡
  Error(const char *err_msg);

  Error(const Error &other);

  const Error &operator=(const Error &rhs);

  ~Error();

  bool IsErr();
  std::string GetMessage();
};

// エラーが発生するかもしれない関数の返り値として利用する｡
// T に参照は利用できない｡なぜならエラーの際に表す値が存在しないから｡
//
// ===== Usage =====
// Result<int> succeed() {  // 成功
//   return 10;
// }
//
// Result<void> succeedVoid() {  // 成功 (返り値がvoid)
//   return Result<void>();
// }
//
// Result<void> fail() {  // エラー
//   return Error("This is error!");
// }
//
// int main(){
//   Result<int> a = succeed();
//   if (a.IsOk()) {
//     std::cout << "val: " << a.Ok() << std::endl;
//   }
//   if (a.IsErr()) {
//     std::cout << "err: " << a.Err().Print() << std::endl;
//   }
// }
template <typename T>
class Result {
 private:
  T val_;
  Error err_;

 public:
  // デフォルトコンストラクタが存在しない場合にエラーを通知したい場合はこのコンストラクタを使う｡
  Result(const T &val, const Error &err) : val_(val), err_(err) {}

  Result(const T &val) : val_(val), err_(false) {}

  // デフォルトコンストラクタが存在しない場合は使えない
  Result(const Error &err) : val_(), err_(err) {}

  bool IsOk() {
    return !err_.IsErr();
  }
  T Ok() {
    assert(IsOk());
    return val_;
  }
  bool IsErr() {
    return err_.IsErr();
  }
  Error Err() {
    assert(IsErr());
    return err_;
  }
};

// T = void の場合はメンバー変数 val_ を保持することはできない｡
//   Ok() も利用できない｡ エラーチェックのみ行うことができる｡
//
// 返り値が void 型の関数の返り値は利用しないことを考えれば適切な挙動である｡
template <>
class Result<void> {
 private:
  Error err_;

 public:
  Result() : err_(false) {}

  Result(const Error &err) : err_(err) {}

  bool IsOk() {
    return !err_.IsErr();
  }
  bool IsErr() {
    return err_.IsErr();
  }
  Error Err() {
    return err_;
  }
};

}  // namespace result

#endif
