#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include <string>
#include "./user.hpp"
#include "./channel.hpp"
#include "TotalDatabase.hpp"


class Server
{
    public:
        Server(int port);
        void run();

    private:
        int                        _listenFd;
        std::vector<struct pollfd> _pfds;       // fd + 이벤트
        std::map<int, User>        _users;      // fd ➔ User
        TotalDatabase<User>        _users2;
        TotalDatabase<Channel>        _users2;
        Channel                    _lobby;      // 단일 채널
        static bool readflag;

        void _setupSocket(int port);
        void _acceptClient();
        void _readLines(User& u, size_t idx);
        void _flushOut(User& u, size_t idx);

        std::string _fdToStr(int fd) const;
};

#endif
