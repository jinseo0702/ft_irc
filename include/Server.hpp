#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
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
#include "Bot.hpp"
#include "DCC.hpp"

class Server {
    public:
        Server(int port, std::string& password);
        void run();
        
        void stop(); 
    private:
        int                        _listenFd;
        std::vector<struct pollfd> _pfds;
        TotalDatabase<User>        _users;
        TotalDatabase<Channel>     _channels;
        
        
        SharedPtr<Channel>         _lobby;
        Password                   _pwd;
        bool                       live;

        
        DCCManager                  _dccManager;
        Bot                         _bot;
        SharedPtr<User>             _botUser;

        
        void _setupSocket(int port);
        void _acceptClient();
        bool _readLines(User& u, size_t idx);
        bool _flushOut(User& u, size_t idx);
        void _disconnectUser(size_t idx);

        std::string _fdToStr(int fd) const;
        void _dispatch(User& u, const Parser& p);

        
        void handleJoin(User& u, const Parser& p);
        void handleNick(User& u, const Parser& p);
        void handleUser(User& u, const Parser& p);
        void handlePart(User& u, const Parser& p);
        void handleQuit(User& u, const Parser& p);
        void handlePrivMsg(User& user, const Parser& parser);
        bool _handleDccPrivMsg(User& user, const Parser& parser);
        void _handleDccSendInPrivMsg(User& user, const std::string& targetNick, std::stringstream& ss);
        void _handleDccAcceptInPrivMsg(User& user, const std::string& targetNick, std::stringstream& ss);
        void _handleRegularPrivMsg(User& user, const Parser& parser);
        void handleNotice(User& u, const Parser& p);
        void handleKick(User& u, const Parser& p);
        void handleInvite(User& u, const Parser& p);
        void handleTopic(User& u, const Parser& p);
        void handleMode(User& u, const Parser& p);
        bool handlePASS(User& u, const Parser& p);
        void handleList(User& u);
        void handleShow(User& u, const Parser& p);

        
        void handleDCCSend(User& u, const Parser& p);
        void handleDCCAccept(User& u, const Parser& p);
        void handleDCCResume(User& u, const Parser& p);
        void handleDCCReject(User& u, const Parser& p);
        
        
        void _initializeBot();
        void _processBotMessages();
        void _handleBotCommands(User& u, const Parser& p);
        
        
        Channel* getChannelByName(const std::string& name);
        User*    getUserByNick(const std::string& nick);
        int      getSamefdUser(const int _pfdsFd);

        void applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             
                         User& src);
};

#endif
