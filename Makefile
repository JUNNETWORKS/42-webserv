NAME := webserv
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror --std=c++98 -pedantic -MMD -MP
# コマンドライン上で DEBUGFLAGS を上書き したいので、CXXFLAGS から分離
DEBUGFLAGS := -g3 -fsanitize=address
INCLUDE := -I srcs

CXXFLAGS += $(DEBUGFLAGS)

SRCS_DIR := srcs
SRCS := $(shell find srcs -type f -name '*.cpp')
OBJS_DIR := objs
OBJS := $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
DEPENDENCIES := $(SRCS:%.cpp=$(OBJS_DIR)/%.d)

.PHONY: all
all: $(NAME)
$(NAME): $(OBJS)
	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $(NAME) $^

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDENCIES)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPENDENCIES)
	$(RM) -r $(OBJS_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

# make -j 実行時、fclean と all が同時に実行されないようにするため
.PHONY: re
re:
	$(MAKE) fclean
	$(MAKE) all

############ GooleTest ############

TEST_DIR    := unit_test
TESTER_NAME := ./tester

GTEST_DIR   := ./google_test

GTEST       := $(GTEST_DIR)/gtest $(GTEST_DIR)/googletest-release-1.11.0
GTEST_MAIN  := $(GTEST_DIR)/googletest-release-1.11.0/googletest/src/gtest_main.cc
GTEST_ALL   := $(GTEST_DIR)/gtest/gtest-all.cc
TEST_SRCS   := $(shell find unit_test -type f -name '*.cpp')
# main() がかぶらないようにmain.cppのみ取り除く
TEST_SRCS   += $(filter-out srcs/server/main.cpp, $(SRCS))
TEST_OBJS   := $(TEST_SRCS:%.cpp=$(OBJS_DIR)/%.o)
TEST_DEPENDENCIES := $(TEST_SRCS:%.cpp=$(OBJS_DIR)/%.d)

-include $(TEST_DEPENDENCIES)

.PHONY: test
test: CXXFLAGS := -I$(SRCS_DIR) -I$(TEST_DIR) --std=c++11 -MMD -MP -I$(GTEST_DIR) $(DEBUGFLAGS)
test: $(TESTER_NAME)
	$(TESTER_NAME)

$(TESTER_NAME): $(GTEST) $(TEST_OBJS)
	# Google Test require C++11
	$(CXX) $(CXXFLAGS) $(GTEST_MAIN) $(GTEST_ALL) \
		-I$(GTEST_DIR) -lpthread \
		$(TEST_OBJS) \
		-o $(TESTER_NAME)

$(GTEST):
	mkdir -p $(GTEST_DIR)
	curl -OL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
	tar -xvzf release-1.11.0.tar.gz googletest-release-1.11.0
	rm -rf release-1.11.0.tar.gz
	python googletest-release-1.11.0/googletest/scripts/fuse_gtest_files.py $(GTEST_DIR)
	mv googletest-release-1.11.0 $(GTEST_DIR)
