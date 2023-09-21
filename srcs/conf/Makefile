CXX = c++
CXXFLAGS = -g -Werror -Wextra -Wall -std=c++98 -pedantic-errors -MMD -MP
NAME = configparser
SRCS = Location.cpp ConfigParser.cpp VirtualServer.cpp #main.cpp
TEST_SRCS = test.cpp
OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

ifdef TEST
OBJS += $(TEST_OBJS)
endif

all: $(NAME)

test:
	make TEST=1

i:
	make TEST=1
	./$(NAME) invalid_test

v:
	make TEST=1
	./$(NAME) valid_test

a:
	make TEST=1
	./$(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS) $(TEST_OBJS)
	$(RM) *.d

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include *.d
