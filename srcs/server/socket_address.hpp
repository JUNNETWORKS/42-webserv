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
  SocketAddress(const struct sockaddr *sockaddr, const int sockaddr_len);
  SocketAddress(const SocketAddress &rhs);
  SocketAddress &operator=(const SocketAddress &rhs);
  ~SocketAddress();

  const struct sockaddr *GetSockaddr() const;
  int GetSockaddrLen() const;

  const std::string &GetIp() const;
  const std::string &GetName() const;
  const std::string &GetPort() const;
};

}  // namespace server
