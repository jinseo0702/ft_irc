CC = clang++
# CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
CFLAGS = -g
RM = rm -rf
MV = mv
# PATH = "."

SRC = ../srcs/server.cpp \
../srcs/user.cpp \
../srcs/channel.cpp \
../srcs/Password.cpp \
../srcs/ChannelData.cpp \
../srcs/Parser.cpp \
../srcs/Rulehandle.cpp \
../srcs/Utils.cpp \
../srcs/UserContainer.cpp \
../srcs/SHA256.cpp \
./PassWordTest.cpp \

OBJS = $(SRC:.cpp=.o)
NAME = test

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
