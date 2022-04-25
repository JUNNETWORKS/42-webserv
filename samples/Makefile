OBJS_DIR := objs

UTILS_DIR := utils
UTILS_SRCS := utils/read_line.cpp utils/utils.cpp utils/inet_sockets.cpp utils/rdwrn.cpp
UTILS_OBJS := $(UTILS_SRCS:%.cpp=$(OBJS_DIR)/%.o)
INCLUDES := -I$(UTILS_DIR)

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) -c $< -o $@

udp: $(UTILS_OBJS)
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp/server.cpp -o server
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp/client.cpp -o client

tcp: $(UTILS_OBJS)
	$(CC) $(INCLUDES) $(UTILS_OBJS) tcp/server.cpp -o server
	$(CC) $(INCLUDES) $(UTILS_OBJS) tcp/client.cpp -o client

udp_iterative_echo: $(UTILS_OBJS)
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp_iterative_echo/server.cpp -o server
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp_iterative_echo/client.cpp -o client
