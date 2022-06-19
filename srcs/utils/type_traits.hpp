#ifndef UTILS_TYPE_TRAITS_HPP_
#define UTILS_TYPE_TRAITS_HPP_

namespace utils {

template <class T>
struct remove_reference {
  typedef T type;
};
template <class T>
struct remove_reference<T&> {
  typedef T type;
};

}  // namespace utils

#endif
