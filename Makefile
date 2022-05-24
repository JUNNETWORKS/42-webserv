CXX := c++
CXXFLAGS := -Wall -Wextra -Werror
CXXFLAGS += --std=c++98
NAME := webserv

SRCS_DIR := srcs
SRCS := $(shell find srcs -type f -name '*.cpp')
OBJS_DIR := objs
OBJS := $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
DEPENDENCIES := $(OBJS:.o=.d)

CXXFLAGS += -I$(SRCS_DIR)

.PHONY: all
all: $(NAME)

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -MMD -MP -o $@

-include $(DEPENDENCIES)


$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPENDENCIES)
	$(RM) -r $(OBJS_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean all
