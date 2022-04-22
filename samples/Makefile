OBJS_DIR := objs

UTILS_DIR := utils
UTILS_SRCS := utils/read_line.cpp
UTILS_OBJS := $(UTILS_SRCS:%.cpp=$(OBJS_DIR)/%.o)
INCLUDES := -I$(UTILS_DIR)


$(UTILS_OBJS): $(UTILS_SRCS)
	@mkdir -p $(@D)
	$(CC) -o $@ -c $^

udp: $(UTILS_OBJS) $(UDP_SAMPLE_SRCS)
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp/server.cpp -o server
	$(CC) $(INCLUDES) $(UTILS_OBJS) udp/client.cpp -o client

tcp: $(UTILS_OBJS) $(TCP_SAMPLE_SRCS)
	$(CC) $(INCLUDES) $(UTILS_OBJS) tcp/server.cpp -o server
	$(CC) $(INCLUDES) $(UTILS_OBJS) tcp/client.cpp -o client
