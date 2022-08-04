#ifndef UTILS_SIGNAL_HPP
#define UTILS_SIGNAL_HPP

#include <signal.h>

namespace utils {

bool set_signal_handler(int signal, sig_t handler, int sa_flags);

}  // namespace utils

#endif
