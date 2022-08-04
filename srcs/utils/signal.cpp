#include <signal.h>

#include <string>

namespace utils {

bool set_signal_handler(int signal, sig_t handler, int sa_flags) {
  struct sigaction act;

  std::memset(&act, 0, sizeof(act));
  act.sa_handler = handler;
  act.sa_flags = sa_flags;
  if (sigemptyset(&act.sa_mask))
    return (false);
  if (sigaddset(&act.sa_mask, signal))
    return (false);
  if (sigaction(signal, &act, NULL))
    return (false);
  return (true);
}

}  // namespace utils
