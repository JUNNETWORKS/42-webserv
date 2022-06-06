#ifndef SERVER_TYPES_HPP_
#define SERVER_TYPES_HPP_

#include <map>
#include <string>

namespace server {

// map[<listen_fd>] = <port>
typedef std::map<int, std::string> ListenFdPortMap;

}  // namespace server

#endif
