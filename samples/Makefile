CXX := g++
OBJS_DIR := objs

UTILS_DIR := utils
UTILS_SRCS := utils/read_line.cpp utils/utils.cpp utils/inet_sockets.cpp utils/rdwrn.cpp
UTILS_OBJS := $(UTILS_SRCS:%.cpp=$(OBJS_DIR)/%.o)
INCLUDES := -I$(UTILS_DIR)

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c $< -o $@

udp: $(UTILS_OBJS)  udp/server.cpp udp/client.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) udp/server.cpp -o server
	$(CXX) $(INCLUDES) $(UTILS_OBJS) udp/client.cpp -o client

tcp: $(UTILS_OBJS)  tcp/server.cpp tcp/client.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) tcp/server.cpp -o server
	$(CXX) $(INCLUDES) $(UTILS_OBJS) tcp/client.cpp -o client

udp_iterative_echo: $(UTILS_OBJS) udp_concurrent_echo/server.cpp udp_concurrent_echo/client.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) udp_iterative_echo/server.cpp -o server
	$(CXX) $(INCLUDES) $(UTILS_OBJS) udp_iterative_echo/client.cpp -o client

tcp_concurrent_echo: $(UTILS_OBJS) tcp_concurrent_echo/server.cpp tcp_concurrent_echo/client.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) tcp_concurrent_echo/server.cpp -o server
	$(CXX) $(INCLUDES) $(UTILS_OBJS) tcp_concurrent_echo/client.cpp -o client

select: $(UTILS_OBJS) select/main.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) select/main.cpp -o a.out

poll: $(UTILS_OBJS) poll/main.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) poll/main.cpp -o a.out

epoll: $(UTILS_OBJS) epoll/main.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) epoll/main.cpp -o a.out

socket_and_cgi: $(UTILS_OBJS) socket_and_cgi/cgi.cpp socket_and_cgi/server.cpp
	$(CXX) $(INCLUDES) $(UTILS_OBJS) socket_and_cgi/cgi.cpp -o cgi-bin/cpp.cgi
	$(CXX) $(INCLUDES) $(UTILS_OBJS) socket_and_cgi/server.cpp -o server
