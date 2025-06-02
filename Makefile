CC = clang++
# CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
CFLAGS = -g
RM = rm -rf

SRC = ./srcs/main.cpp \
./srcs/server.cpp \
./srcs/user.cpp \
./srcs/channel.cpp \
./srcs/Password.cpp \
./srcs/ChannelData.cpp \

OBJS = $(SRC:.cpp=.o)
NAME = ft_irc

all : $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(OBJS) -o $(NAME)

%.o : %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@

clean :
	@$(RM) $(OBJS)

fclean :
	@$(RM) $(OBJS) $(NAME)

re : 
	@make fclean
	@make all

.PHONY: all clean fclean re
