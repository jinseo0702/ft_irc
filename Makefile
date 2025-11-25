CXX = clang++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CXXFLAGS_DEBUG = -g3 -O0 -ggdb -fno-inline -fno-omit-frame-pointer -fstandalone-debug
CXXFLAGS_RELEASE = -O2 -DNDEBUG -fomit-frame-pointer -ffast-math
CXXFLAGS_OPTIMIZED = -O3 -DNDEBUG -fomit-frame-pointer -ffast-math -march=native
RM = rm -rf

SRC = ./srcs/BotCore.cpp \
./srcs/Channel.cpp \
./srcs/ChannelData.cpp \
./srcs/DCCManager.cpp \
./srcs/DCCSession.cpp \
./srcs/main.cpp \
./srcs/Parser.cpp \
./srcs/Password.cpp \
./srcs/Rulehandle.cpp \
./srcs/ServerBot.cpp \
./srcs/Server.cpp \
./srcs/ServerDCC.cpp \
./srcs/SHA256.cpp \
./srcs/signal.cpp \
./srcs/User.cpp \
./srcs/Utils.cpp \
./srcs/BotCommands.cpp

OBJS = $(SRC:.cpp=.o)
NAME = ircserv

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

# 디버그 타겟
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: clean $(NAME)

# 릴리즈 타겟
release: CXXFLAGS += $(CXXFLAGS_RELEASE) 
release: clean $(NAME)

# 최적화 타겟
optimized: CXXFLAGS += $(CXXFLAGS_OPTIMIZED)
optimized: clean $(NAME)

# 프로파일링 타겟
profile: CXXFLAGS += -pg -O2
profile: clean $(NAME)

.PHONY: all clean fclean re debug release optimized profile
