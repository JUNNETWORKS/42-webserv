#ifndef UNIT_TEST_EXPECT_RESULT_HPP_
#define UNIT_TEST_EXPECT_RESULT_HPP_

#include <gtest/gtest.h>

#include "result/result.hpp"
#include "stdio.h"

using namespace result;

template <typename T>
void EXPECT_RESULT_IS_OK(Result<T> res1) {
  EXPECT_TRUE(res1.IsOk());
}

template <typename T>
void EXPECT_RESULT_OK_EQ(Result<T> res1, Result<T> res2) {
  EXPECT_EQ(res1.Ok(), res2.Ok());
}

template <typename T>
void EXPECT_RESULT_IS_ERR(Result<T> res1) {
  EXPECT_TRUE(res1.IsErr());
}

template <typename T>
void EXPECT_RESULT_ERR_EQ(Result<T> res1, Result<T> res2) {
  Error err1 = res1.Err();
  Error err2 = res2.Err();
  EXPECT_EQ(err1.IsErr(), err2.IsErr());
  EXPECT_EQ(err1.GetMessage(), err2.GetMessage());
}

#endif
