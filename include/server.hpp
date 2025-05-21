#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include <string>
#include "./user.hpp"
#include "./channel.hpp"

class Server
{
    public:
        Server(int port);
        void run();

    private:
        int                        _listenFd;
        std::vector<struct pollfd> _pfds;       // fd + 이벤트
        std::map<int, User>        _users;      // fd ➔ User
        Channel                    _lobby;      // 단일 채널

        void _setupSocket(int port);
        void _acceptClient();
        void _readLines(User& u, size_t idx);
        void _flushOut(User& u, size_t idx);

        std::string _fdToStr(int fd) const;
};

#endif
