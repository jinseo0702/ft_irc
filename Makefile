# CC = clang++
# # CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
# CXXFLAGS_DEBUG = -g3 -O0 -ggdb -fno-inline -fno-omit-frame-pointer
# CFLAGS = -g
# RM = rm -rf

# SRC = ./srcs/main.cpp \
# ./srcs/server.cpp \
# ./srcs/user.cpp \
# ./srcs/channel.cpp \
# ./srcs/Password.cpp \
# ./srcs/ChannelData.cpp \
# ./srcs/Parser.cpp \
# ./srcs/Rulehandle.cpp \
# ./srcs/Utils.cpp \
# ./srcs/UserContainer.cpp \

# OBJS = $(SRC:.cpp=.o)
# NAME = ft_irc

# all : $(NAME)

# $(NAME): $(OBJS)
# 	@$(CC) $(OBJS) -o $(NAME)

# %.o : %.cpp
# 	@$(CC) $(CFLAGS) -c $< -o $@

# clean :
# 	@$(RM) $(OBJS)

# fclean :
# 	@$(RM) $(OBJS) $(NAME)

# re : 
# 	@make fclean
# 	@make all

# debug: CXXFLAGS = $(CXXFLAGS_DEBUG)

# debug: $(NAME)

# .PHONY: all clean fclean re debug


CXX = clang++
# CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CXXFLAGS_DEBUG = -g3 -O0 -ggdb -fno-inline -fno-omit-frame-pointer -fstandalone-debug
CXXFLAGS_RELEASE = -O2 -DNDEBUG
RM = rm -rf

SRC = ./srcs/main.cpp \
./srcs/Server.cpp \
./srcs/User.cpp \
./srcs/Channel.cpp \
./srcs/Password.cpp \
./srcs/ChannelData.cpp \
./srcs/Parser.cpp \
./srcs/Rulehandle.cpp \
./srcs/Utils.cpp \
./srcs/UserContainer.cpp \
./srcs/SHA256.cpp

OBJS = $(SRC:.cpp=.o)
NAME = ft_irc

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: 
	@make fclean
	@make all

# 디버그 타겟 수정
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: clean $(NAME)

# 릴리즈 타겟
release: CXXFLAGS += $(CXXFLAGS_RELEASE) 
release: clean $(NAME)

.PHONY: all clean fclean re debug release