#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include "User.hpp"
#include "Channel.hpp"
#include "TotalDatabase.hpp"
#include "SharedPtr.hpp"
#include "Password.hpp"
#include "Parser.hpp"
#include "Rulehandle.hpp"
#include "DCC.hpp"
#include "Bot.hpp"


class Channel;
class User;

namespace {
    const int BUFFER_SIZE = 2048;
    const int STDIN_FD = 0;
    const int SUPER_USER_FD = 777;
    const int ERROR_ID = -999;
    const int INACTIVE_FD = -2;
    const int BACKLOG = 20;
    const int MAX_USER_LIMIT = 10000;
}

class Server {
    public:
        Server(int port, std::string& password);
        void run();
        //signal fuc
        void stop(); 
    private:
        int                        _listenFd;
        std::vector<struct pollfd> _pfds;
        TotalDatabase<User>        _users;
        TotalDatabase<Channel>     _channels;
        
        // lobby는 0번 채널로 항상 존재
        SharedPtr<Channel>         _lobby;
        Password                   _pwd;
        bool                       live;

        DCCManager                 _dccManager;
        Bot                        _bot;
        SharedPtr<User>            _botUser;

        // core methods
        void _setupSocket(int port);
        void _acceptClient();
        void _readLines(User& u, size_t idx);
        void _flushOut(User& u, size_t idx);
        void _disconnectUser(size_t idx);

        std::string _fdToStr(int fd) const;
        void _dispatch(User& u, const Parser& p);

        // Command Handlers
        void handleJoin(User& u, const Parser& p);
        void handleNick(User& u, const Parser& p);
        void handleUser(User& u, const Parser& p);
        void handlePart(User& u, const Parser& p);
        void handleQuit(User& u, const Parser& p);
        void handlePrivMsg(User& u, const Parser& p);
        void handleNotice(User& u, const Parser& p);
        void handleKick(User& u, const Parser& p);
        void handleInvite(User& u, const Parser& p);
        void handleTopic(User& u, const Parser& p);
        void handleMode(User& u, const Parser& p);
        bool handlePASS(User& u, const Parser& p);
        void handleList(User& u);
        void handleShow(User& u, const Parser& p);

        // DCC 명령어 핸들러 추가
        void handleDCCSend(User& u, const Parser& p);
        void handleDCCAccept(User& u, const Parser& p);
        void handleDCCResume(User& u, const Parser& p);
        void handleDCCReject(User& u, const Parser& p);

        // 봇 관련 메서드
        void _initializeBot();
        void _processBotMessages();
        void _handleBotCommands(User& u, const Parser& p);

        // etc utils
        Channel* getChannelByName(const std::string& name);
        User*    getUserByNick(const std::string& nick);
        int      getSamefdUser(const int _pfdsFd);

        void applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,
                         User& src);
};

#endif
