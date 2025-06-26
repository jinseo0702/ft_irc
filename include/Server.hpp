#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include <string>
#include "User.hpp"
#include "Channel.hpp"
#include "TotalDatabase.hpp"
#include "SharedPtr.hpp"
#include "Password.hpp"
#include "Parser.hpp"
#include "Rulehandle.hpp"

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

        // etc utils
        Channel* getChannelByName(const std::string& name);
        User*    getUserByNick(const std::string& nick);
        int      getSamefdUser(const int _pfdsFd);

        void applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             // true = +o, false = -o
                         User& src);
};

#endif
