#include "server/event_loop.hpp"

#include "config/config.hpp"
#include "server/epoll.hpp"
#include "utils/error.hpp"
#include "utils/inet_sockets.hpp"

namespace server {

int StartEventLoop(Epoll &epoll) {
  // イベントループ
  while (1) {
    std::vector<FdEventEvent> timeouts = epoll.RetrieveTimeouts();
    printf("timeout count: %ld\n", timeouts.size());
    for (std::vector<FdEventEvent>::const_iterator it = timeouts.begin();
         it != timeouts.end(); ++it) {
      FdEvent *fde = it->fde;
      unsigned int events = it->events;
      InvokeFdEvent(fde, events, &epoll);
    }

    Result<std::vector<FdEventEvent> > result = epoll.WaitEvents(10);
    if (result.IsErr()) {
      utils::ErrExit("WaitEvents");
    }

    std::vector<FdEventEvent> fdees = result.Ok();
    for (std::vector<FdEventEvent>::const_iterator it = fdees.begin();
         it != fdees.end(); ++it) {
      FdEvent *fde = it->fde;
      unsigned int events = it->events;
      InvokeFdEvent(fde, events, &epoll);
    }
  }
}

}  // namespace server
