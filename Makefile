CXX = clang++
# CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CXXFLAGS_DEBUG = -g3 -O0 -ggdb -fno-inline -fno-omit-frame-pointer -fstandalone-debug
CXXFLAGS_RELEASE = -O2 -DNDEBUG
RM = rm -rf

# 분리된 소스 파일들
SRC = ./srcs/main.cpp \
./srcs/ServerCore.cpp \
./srcs/ServerCommands.cpp \
./srcs/ServerBot.cpp \
./srcs/ServerDCC.cpp \
./srcs/User.cpp \
./srcs/Channel.cpp \
./srcs/Password.cpp \
./srcs/ChannelData.cpp \
./srcs/Parser.cpp \
./srcs/Rulehandle.cpp \
./srcs/Utils.cpp \
./srcs/UserContainer.cpp \
./srcs/SHA256.cpp \
./srcs/signal.cpp \
./srcs/DCCSession.cpp \
./srcs/DCCManager.cpp \
./srcs/BotCore.cpp \
./srcs/BotCommands.cpp

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