CXX = c++
CXXFLAGS = -Werror -Wextra -Wall -std=c++98 -pedantic-errors -MMD -MP
NAME = webserv
SRCS = main.cpp Socket.cpp Logger.cpp Client.cpp EventLoop.cpp Epoll.cpp

CONF_DIR = conf/
CONF_FILES = Location.cpp ConfigParser.cpp VirtualServer.cpp
CONF_SRCS = $(addprefix $(CONF_DIR), $(CONF_FILES))
HTTP_DIR = http/
HTTP_FILES = HttpRes.cpp HttpReq.cpp Cgi.cpp
HTTP_SRCS = $(addprefix $(HTTP_DIR), $(HTTP_FILES))

OBJS = $(SRCS:.cpp=.o)
CONF_OBJS = $(CONF_SRCS:.cpp=.o)
HTTP_OBJS = $(HTTP_SRCS:.cpp=.o)
OBJS += $(CONF_OBJS)
OBJS += $(HTTP_OBJS)

all: $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS)
	$(RM) **/*.d

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include *.d
