#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace server {

class SocketAddress {
 private:
  struct sockaddr *sockaddr_;
  int sockaddr_len_;

 public:
  SocketAddress();
  SocketAddress(const struct sockaddr *sockaddr, const socklen_t sockaddr_len);
  SocketAddress(const SocketAddress &rhs);
  SocketAddress &operator=(const SocketAddress &rhs);
  ~SocketAddress();

  void SetSockaddr(const struct sockaddr *sockaddr,
                   const socklen_t sockaddr_len);
  const struct sockaddr *GetSockaddr() const;
  socklen_t GetSockaddrLen() const;

  const std::string &GetIp() const;
  const std::string &GetName() const;
  const std::string &GetPort() const;
};

}  // namespace server
