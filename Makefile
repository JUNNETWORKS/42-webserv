CXX := c++
NAME := webserv

SRCS_DIR := srcs
SRCS := $(shell find srcs -type f -name '*.cpp')
OBJS_DIR := objs
OBJS := $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
DEPENDENCIES := $(OBJS:.o=.d)

CXXFLAGS := -I$(SRCS_DIR)

GTEST_DIR   := ./google_test
ifneq ($(filter $(strip $(MAKECMDGOALS)),test),)
	CXXFLAGS += --std=c++11
	CXXFLAGS += -I$(GTEST_DIR)
	CXXFLAGS += -g -fsanitize=address
else
	CXXFLAGS += --std=c++98
	CXXFLAGS += -Wall -Wextra -Werror
endif

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

############ GooleTest ############

TESTER_NAME := ./tester

GTEST       := $(GTEST_DIR)/gtest $(GTEST_DIR)/googletest-release-1.11.0
GTEST_MAIN  := $(GTEST_DIR)/googletest-release-1.11.0/googletest/src/gtest_main.cc
GTEST_ALL   := $(GTEST_DIR)/gtest/gtest-all.cc
TEST_SRCS   := $(shell find unit_test -type f -name '*.cpp')
# main() がかぶらないようにmain.cppのオブジェクトファイルのみ取り除く
TEST_OBJS  := $(filter-out objs/srcs/server/main.o, $(OBJS)) $(TEST_SRCS:%.cpp=$(OBJS_DIR)/%.o)
TEST_DEPENDENCIES \
         := $(TEST_OBJS:%.o=%.d)

-include $(TEST_DEPENDENCIES)



.PHONY: test
test: $(GTEST) $(TEST_OBJS)
	# Google Test require C++11
	$(CXX) $(CXXFLAGS) $(GTEST_MAIN) $(GTEST_ALL) \
		-I$(GTEST_DIR) -lpthread \
		$(TEST_OBJS) \
		-o $(TESTER_NAME)
	$(TESTER_NAME)

$(GTEST):
	mkdir -p $(GTEST_DIR)
	curl -OL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
	tar -xvzf release-1.11.0.tar.gz googletest-release-1.11.0
	rm -rf release-1.11.0.tar.gz
	python googletest-release-1.11.0/googletest/scripts/fuse_gtest_files.py $(GTEST_DIR)
	mv googletest-release-1.11.0 $(GTEST_DIR)
