#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Password.hpp"
#include "../include/SharedPtr.hpp"
#include <netdb.h>
#include <arpa/inet.h>


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


Server::Server(int port, std::string& password)
    : _listenFd(-1)
{
    _setupSocket(port);

    
    SharedPtr<Channel> lobby(new Channel());   

    
    this->_channels.addUserWithId(lobby);

    
    this->_lobby = lobby;

    struct pollfd pfd = { 
        STDIN_FILENO, POLLIN, 0 
    };
 
    _pfds.push_back(pfd);
    User* rawUser = new User(SUPER_USER_FD);                                                 

    _users.addUserWithId(rawUser);

    SharedPtr<User> uPtr = _users.returnSecond(rawUser->getId());

    _lobby->addUser(uPtr);
    rawUser->addOutbox(":server NOTICE * :Welcome to Operator Ur in Lobby\r\n");
    
    _initializeBot();
    
    _pwd.setisPasswordSet(true);
    _pwd.setPwd(password);

    std::cout << "Listening on port " << port
              << " (password: " << password << "), #lobby created\n";
    this->live = true;
}



void Server::_setupSocket(int port)
{
    this->_listenFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (this->_listenFd < 0)
        throw std::runtime_error("socket error");

    int yes = 1;
    setsockopt(this->_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in a; std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(port);

    if (bind(this->_listenFd, (struct sockaddr*)&a, sizeof(a)) < 0)
        throw std::runtime_error("bind error");
    if (listen(this->_listenFd, BACKLOG) < 0)
        throw std::runtime_error("listen error");

    struct pollfd pfd;
    pfd.fd = this->_listenFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_pfds.push_back(pfd);
}


void Server::run()
{
    while (true)
    {
        if (Sig::stopRequested()) {
            stop();                 
            break;                  
        }
        if (poll(&this->_pfds[0], this->_pfds.size(), -1) < 0){
            if (errno == EINTR)      
                continue;            
            throw std::runtime_error("poll error");
        }   
        if (this->live == false){
            break;
        }
        
        if (this->_pfds[0].revents & POLLIN){
            _acceptClient();
        }

        
        size_t i = 0;
        while (++i < this->_pfds.size()){
            int id = getSamefdUser(this->_pfds[i].fd);
            if (id == -999){
                continue;
            }
            SharedPtr<User> user = _users.returnSecond(id);
            if (this->_pfds[i].revents & POLLIN){
                _readLines(*user, i);
            }
            if (i < this->_pfds.size() && (this->_pfds[i].revents & POLLOUT)){
                _flushOut(*user, i);
            }
        }
        
        _dccManager.processDCCTransfers();
        
        
        _processBotMessages();
    }
    for (size_t i = 2; i < this->_pfds.size(); i++){
        int id = getSamefdUser(this->_pfds[i].fd);
        if (id == -999){
                continue;
            }
        SharedPtr<User> user = _users.returnSecond(id);
        if (i < this->_pfds.size() && (this->_pfds[i].revents & POLLOUT)){
                _flushOut(*user, i);
            }
    }
    for (int k = static_cast<int>(_pfds.size()) - 1; k >= 2; --k)
        _disconnectUser(k);
    if (_listenFd >= 0)
        close(_listenFd);   
    return ;
}

void Server::_acceptClient()
{
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int cfd = accept(_listenFd, (struct sockaddr*)&client_addr, &client_len);
    if (cfd < 0)        
        return;

    struct pollfd pfd = { cfd, POLLIN, 0 };
    _pfds.push_back(pfd);

    
    SharedPtr<User> u(new User(cfd));     
    u->setHostname(inet_ntoa(client_addr.sin_addr));
    std::cout << "New connection from: " << u->getHostname() << std::endl;
    
    struct sockaddr_in server_sock_addr;
    socklen_t server_sock_len = sizeof(server_sock_addr);
    if (getsockname(cfd, (struct sockaddr*)&server_sock_addr, &server_sock_len) == 0) {
        u->setServerAddress(inet_ntoa(server_sock_addr.sin_addr));
    }
    
    _users.addUserWithId(u);                        

    
    _lobby->addUser(u);                   

    
    
    
    
    _pfds.back().events |= POLLOUT;
}


void Server::_disconnectUser(size_t idx)
{
    int fd = _pfds[idx].fd;

    User* u = NULL;
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it){
        if (it->second->getFd() == fd){
            u = it->second.get();
            break;
        }
    }
    if (!u){
        return;
    }
    u->setActive(false);
    u->setFd(INACTIVE_FD);

    close(fd);
    _pfds.erase(_pfds.begin() + idx);
}



void Server::_readLines(User& u, size_t idx){

    ssize_t n;
    char buf[BUFFER_SIZE] = {0,};
    if (u.getFd() == STDIN_FD){
        std::string super;
        if (!std::getline(std::cin, super))
            return;  
        n = super.copy(buf, sizeof(buf) - 1);
        int len = std::strlen(buf);
        buf[len] = '\n';
        n += 1;
    }
    else{

        n = recv(u.getFd(), buf, sizeof(buf)-1, 0);
        if (n == 0){
            _disconnectUser(idx);
            return;
        }
        if (n < 0)
        {
            
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;                            

            
            _disconnectUser(idx);
            return;
        }
    }
    
    u.getReferIbuf().append(buf, n);
    
    size_t pos;
    while ((pos = u.getReferIbuf().find('\n')) != std::string::npos)
    {
        std::string line = u.getReferIbuf().substr(0, pos);
        if (!line.empty() && line[line.size()-1] == '\r')
        line.erase(line.size()-1, 1);
        u.getReferIbuf().erase(0, pos + 1);
        
        Parser p = Parser::parse(line);
        _dispatch(u, p);
    }
    std::cout << "User ["<< u.getId() <<"] insert " << buf << std::endl;
    for (size_t i = 2; i < this->_pfds.size(); ++i){
            this->_pfds[i].events |= POLLOUT;
    }
}

void Server::_flushOut(User& u, size_t idx)
{
    while (!u.getOutbox().empty())
    {
        const std::string m = u.getOutbox().front();
        ssize_t n = send(u.getFd(), m.c_str(), m.size(), 0);
        if (n == (ssize_t)m.size())
            u.getReferOutbox().pop();
        else
            break;
    }
    if (u.getOutbox().empty())
        this->_pfds[idx].events &= ~POLLOUT;
}


std::string Server::_fdToStr(int fd) const {
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}


void Server::_dispatch(User& user, const Parser& parser)
{
    Rulehandle::Mypair cmdinfo = Rulehandle::checkCommand(parser);
    user_role cmd = cmdinfo.second;

    if (Rulehandle::isError(cmd)) {
        
        user.addOutbox(":server 421 " + user.getNickName() + " " + parser.getCommand() + " :Unknown command\r\n");
        return;
    }

    if (user.getNewby() == 0){
        switch (cmd){
            case PASS:
                if (handlePASS(user, parser) == false){
                    return ;
                }
                else{
                    user.addOutbox(":server PassWord is Correct\r\n");
                    return ;
                }
                break;
            case QUIT:
                handleQuit(user, parser);
                break;
            default:
                user.addOutbox(":server SET PASSWORD plz\r\n");
                break;
        }
        return ;
    }
    
    if (user.is_newby()){
        switch (cmd){
        case NICK:      handleNick(user, parser);    break;
        case USER:      handleUser(user, parser);    break;
        case QUIT:      handleQuit(user, parser);    break;
        default:
            user.addOutbox(":server SET UserAndNick plz\r\n");
            break;
        }
        return ;
    }

    if (user.getActive() == false){
        user.addOutbox(":server ERR_FATAL\r\n");
        return ;
    }

    switch(cmd) {
        case JOIN:      handleJoin(user, parser);    break;
        case NICK:      handleNick(user, parser);    break;
        case USER:      handleUser(user, parser);    break;
        case PART:      handlePart(user, parser);    break;
        case QUIT:      handleQuit(user, parser);    break;
        case PRIVMSG:   
            handlePrivMsg(user, parser);
            _handleBotCommands(user,parser);
            break;
        case NOTICE:    handleNotice(user, parser);  break;
        case KICK:      handleKick(user, parser);    break;
        case INVITE:    handleInvite(user, parser);  break;
        case TOPIC:     handleTopic(user, parser);   break;
        case MODE:      handleMode(user, parser);    break;
        case LIST:      handleList(user);    break;
        case SHOW:      handleShow(user, parser);    break;
        default:
            user.addOutbox(":server 421 " + user.getNickName() + " " + parser.getCommand() + " :Unknown command\r\n");
            break;
    }
}

void Server::handleJoin(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if (params.empty()) {
        user.addOutbox(":server 461 " + user.getNickName() + " JOIN :Not enough parameters\r\n");
        return;
    }

    
    std::vector<std::string> channelNames, channelKeys;
    {
        std::istringstream chiss(params[0]);
        std::string channel;
        while (std::getline(chiss, channel, ',')) {
            if (!channel.empty())
                channelNames.push_back(channel);
        }
        if (params.size() >= 2) {
            std::istringstream keyss(params[1]);
            std::string key;
            while (std::getline(keyss, key, ',')) {
                channelKeys.push_back(key);
            }
        }
    }

    for (size_t i = 0; i < channelNames.size(); ++i)
    {
        std::string& channelName = channelNames[i];
        if (!Utils::is_channel(channelName)) {
            user.addOutbox(":server 476 " + user.getNickName() + " " + channelName + " :Bad Channel Mask\r\n");
            continue;
        }
        SharedPtr<Channel> channel;
        for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it) {
            SharedPtr<Channel> temp = it->second;
            if (temp->getChannelName() == channelName) {
                channel = it->second;
                break;
            }
        }
        if (!channel.is_valid()){
            int id = 0;
            id = this->_channels.addUserWithId(new Channel());
            channel = this->_channels.returnSecond(id);
            channel->setName(channelName);
            if (i < channelKeys.size() && Utils::is_key(channelKeys[i]))
                channel->setPwdset(true, channelKeys[i]);
                        
            std::cout << "[NEW CHANNEL] " << channelName << " created" << std::endl;
        }
        
        if (channel->getPwdSet()) {
            std::string pass = (i < channelKeys.size()) ? channelKeys[i] : "";
        
            
            if (pass.empty()) {
                user.numeric(475, channelName + " :Cannot join channel (+k)"); 
                continue;
            }
            
            if (!Utils::is_key(pass)) {
                user.numeric(467, channelName + " :Bad key format");           
                continue;
            }
            
            if (channel->checkPassword(pass) == false) {
                user.numeric(475, channelName + " :Wrong key");                
                continue;
            }
        }
        int changeId = channel->changeServerIdtoChannel(user.getId());
        if (channel->isInviteOnly() &&
            !channel->hasUserById(user.getId()) &&   
            !channel->isInvited(changeId))       
        {
            user.numeric(473, channelName + " :Cannot join channel (+i)");
            continue;            
        }
        channel->removeInvite(changeId);
        
        std::cout << "[JOIN TRY] " << user.getNickName() << " -> " << channelName << std::endl;
        SharedPtr<User> userPtr;
        userPtr = this->_users.returnSecond(user.getId());
        if (!userPtr.is_valid()){
            user.addOutbox(":server 476 " + user.getNickName() + " " + channelName + " :Bad Channel Mask\r\n");
            return ;
        }
        if (channel->hasUserById(user.getId())) {
            user.numeric(ERR_FATAL,
            channelName + " :is already on channel");
            continue;
        }
        channel->addUser(userPtr);
        channel->setIsActive();

        if (_botUser.is_valid())
                channel->addUser(_botUser);

        
        std::string joinMsg = ":" + user.getNickName() + "!" + user.getUserName() + "@localhost JOIN " + channelName + "\r\n";
        channel->broadcast(joinMsg, NULL);
    }
}




void Server::handleNick(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if(parser.parmsCnt() > 1){
        user.addOutbox(":server 461 " + user.getNickName() + " NICK :Not enough parameters\r\n");
        return ;
    }
    if (params.empty() || params[0].empty()) {
        user.addOutbox(":server 431 " + user.getNickName() + " :No nickname given\r\n");
        return;
    }

    std::string newNick = params[0];

    
    if (!Utils::is_nickname(newNick)) {
        user.addOutbox(":server 432 " + user.getNickName() + " " + newNick + " :Erroneous nickname\r\n");
        return;
    }

    
    bool nickInUse = false;
    for (TotalDatabase<User>::const_it uit = _users.begin(); uit != _users.end(); ++uit) {
        if (uit->second->getNickName() == newNick) {
            nickInUse = true;
            break;
        }
    }
    if (nickInUse) {
        user.addOutbox(":server 433 " + user.getNickName() + " " + newNick + " :Nickname is already in use\r\n");
        return;
    }

    
    std::string oldNick = user.getNickName();
    user.setNickName(newNick);
    user.setNewby(NICK);
    if (user.getActive() == false && user.getFd() > 3 && (user.getNewby() == 103)){
        user.setActive(true);
        user.addOutbox(user.getNickName());
        user.addOutbox(" :Welcome to the Internet Relay Network ");
        user.addOutbox(user.getNickName());
        user.addOutbox("\r\n");
    }

    
    
    for (TotalDatabase<Channel>::it chit = _channels.begin(); chit != _channels.end(); ++chit) {
        Channel* ch = chit->second.get();
        
        for (Channel::UserIt uit = ch->userBegin(); uit != ch->userEnd(); ++uit) {
            SharedPtr<ChannelData> chData = uit->second;
            if (chData->getWho() == &user) {
                std::string notice;
                if (!oldNick.empty()) {
                    notice = ":" + oldNick + " NICK " + newNick + "\r\n";
                } else {
                    notice = ":" + newNick + " NICK " + newNick + "\r\n";
                }
                ch->broadcast(notice, NULL); 
                break; 
            }
        }
    }

    
    if (oldNick.empty()) {
        user.addOutbox(":" + newNick + " NICK " + newNick + "\r\n");
    }
}




void Server::handleUser(User& u, const Parser& p)
{
    const std::vector<std::string>& params = p.getParams();

    
    if (params.size() > 4) {
        u.addOutbox(":server 461 " + u.getNickName() + " USER :Not enough parameters\r\n");
        return;
    }

    
    if (!u.getUserName().empty()) {
        u.addOutbox(":server 462 " + u.getNickName() + " :You may not reregister\r\n");
        return;
    }

    
    std::string username = params[0];
    u.setUserName(username);
    u.setNewby(USER);
    if (u.getActive() == false && u.getFd() > 3 && (u.getNewby() == 103)){
        u.setActive(true);
        u.addOutbox(":server 001 " + u.getNickName() + " :Welcome to the Internet Relay Network\r\n");
    }

    
    std::string msg = ":server NOTICE * :Username set to " + username + "\r\n";
    u.addOutbox(msg);
}



void Server::handlePart(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if (params.empty()) {
        user.addOutbox(":server 461 " + user.getNickName() + " PART :Not enough parameters\r\n");
        return;
    }

    
    std::istringstream iss(params[0]);
    std::string channelName;
    while (std::getline(iss, channelName, ',')) {
        Channel* ch = getChannelByName(channelName); 
        if (!ch || !ch->getIsActive()) {
            user.addOutbox(":server 403 " + user.getNickName() + " " + channelName + " :No such channel\r\n");
            continue;
        }
        int changeId = ch->changeServerIdtoChannel(user.getId());
        
        if (!ch->hasUser(changeId)) {
            user.addOutbox(":server 442 " + user.getNickName() + " " + channelName + " :You're not on that channel\r\n");
            continue;
        }

        
        std::string msg = ":" + user.getNickName() + "!" + user.getUserName()
                        + "@localhost PART " + channelName + "\r\n";
        ch->broadcast(msg, NULL); 

        
        ch->eraseUser(changeId);

        
        user.addOutbox(msg);

        
        if (ch->getUserCount() == 0) {
            ch->setInactive(); 
        }
        else
            ch->ensureOneOp();
    }
}



void Server::handleQuit(User& user, const Parser& parser)
{
    if (user.getFd() == STDIN_FD){
        Parser p = Parser::parse("privmsg #lobby :Sever is Down bye bye");
        handlePrivMsg(user, p);
        this->live = false;
        return;
    }
    
    std::string quitMessage = "Client Quit";
    if (!parser.getParams().empty()) {
        quitMessage = parser.getParams()[0];
    }

    
    for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it) {
        SharedPtr<Channel> chPtr = it->second;
        if (!chPtr.is_valid()) 
            continue;
        Channel* ch = chPtr.get();

        int changeId = ch->changeServerIdtoChannel(user.getId());
        
        if (!ch->getIsActive() || !ch->hasUser(changeId))
            continue;

        
        std::string msg =
            ":" + user.getNickName() +
            "!" + user.getUserName() +
            "@localhost QUIT :" + quitMessage + "\r\n";
        ch->broadcast(msg, &user);

        
        ch->eraseUser(changeId);
        if (ch->getUserCount() == 0) {
            ch->setInactive();
        }
        else
            ch->ensureOneOp(); 
    }

    
    user.addOutbox("ERROR :Closing Link: " + user.getNickName() +
                   " (" + quitMessage + ")\r\n");

    
    for (size_t i = 1; i < _pfds.size(); ++i)
    {
    if (_pfds[i].fd == user.getFd()) {
        _disconnectUser(i);      
        break;
    }
}
}





void Server::handlePrivMsg(User& user, const Parser& parser)
{
    
    if (parser.getParams().size() < 2) {
        user.numeric(412, ":No text to send"); 
        return;
    }

    
    
    if (_handleDccPrivMsg(user, parser)) {
        return;
    }

    
    _handleRegularPrivMsg(user, parser);
}

bool Server::_handleDccPrivMsg(User& user, const Parser& parser)
{
    const std::string& dccPayload = parser.getParams()[1];

    
    if (dccPayload.rfind(":DCC", 0) != 0) {
        return false;
    }

    std::string targetNick = parser.getParams()[0];
    std::stringstream ss(dccPayload.substr(1)); 
    std::string keyword, verb;
    ss >> keyword >> verb; 

    if (keyword == "DCC") {
        if (verb == "SEND") {
            _handleDccSendInPrivMsg(user, targetNick, ss);
        } else if (verb == "ACCEPT") {
            _handleDccAcceptInPrivMsg(user, targetNick, ss);
        }
    }
    
    
    return true;
}

void Server::_handleDccSendInPrivMsg(User& user, const std::string& targetNick, std::stringstream& ss)
{
    std::string filename, filesize;
    ss >> filename >> filesize;

    if (filename.empty() || filesize.empty()) {
        user.numeric(461, "PRIVMSG DCC SEND :Not enough parameters");
        return;
    }

    std::vector<std::string> dccParams;
    dccParams.push_back(targetNick);
    dccParams.push_back(filename);
    dccParams.push_back(filesize);

    std::pair<bool, std::string> result = _dccManager.handleDCCSend(user, dccParams);

    if (result.first) { 
        User* receiver = getUserByNick(targetNick);
        if (receiver) {
            receiver->addOutbox(result.second);
        } else {
            user.numeric(401, targetNick + " :No such nick");
        }
    } else { 
        user.addOutbox(result.second);
    }
}

void Server::_handleDccAcceptInPrivMsg(User& user, const std::string& targetNick, std::stringstream& ss)
{
    std::string filename, ip, port, size;
    ss >> filename >> ip >> port >> size;

    if (filename.empty() || ip.empty() || port.empty() || size.empty()) {
        user.numeric(461, "PRIVMSG DCC ACCEPT :Not enough parameters");
        return;
    }

    std::vector<std::string> dccParams;
    dccParams.push_back(filename);
    dccParams.push_back(ip);
    dccParams.push_back(port);
    dccParams.push_back(size);
    dccParams.push_back(targetNick); 

    _dccManager.handleDCCAccept(user, dccParams);
}

void Server::_handleRegularPrivMsg(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    std::string target = params[0];
    std::string text = params[1];

    for (size_t i = 2; i < params.size(); ++i) {
        text += " " + params[i];
    }
    if (text[0] != ':') text = ":" + text;

    if (Utils::is_channel(target)) { 
        Channel* ch = getChannelByName(target);
        if (!ch) {
            user.numeric(403, target + " :No such channel");
            return;
        }
        if (!ch->hasUserGetServerId(user.getId())) {
            user.numeric(442, target + " :You're not on that channel");
            return;
        }
        std::string msg = ":" + user.fullPrefix() + " PRIVMSG " + target + " " + text + "\r\n";
        ch->broadcast(msg, &user);
    } else if (Utils::is_nickname(target)) { 
        User* dest = getUserByNick(target);
        if (!dest) {
            user.numeric(401, target + " :No such nick");
            return;
        }
        std::string msg = ":" + user.fullPrefix() + " PRIVMSG " + target + " " + text + "\r\n";
        dest->addOutbox(msg);
    }
}



void Server::handleNotice(User& user, const Parser& parser)
{
    const std::vector<std::string>& in = parser.getParams();
    if (in.size() < 2)
        return;                   

    
    std::string text = in[1];
    for (size_t i = 2; i < in.size(); ++i)
        text += " " + in[i];
    if (text.empty())
        return;                    
    if (text[0] != ':')
        text = ":" + text;       

    
    std::istringstream tss(in[0]);
    std::string target;

    while (std::getline(tss, target, ','))       
    {
        if (target.empty())
            continue;

        std::string msg = ":" + user.fullPrefix() +
                          " NOTICE " + target + " " + text + "\r\n";

        
        if (Utils::is_channel(target))
        {
            Channel* ch = getChannelByName(target);
            int changeId = ch->changeServerIdtoChannel(user.getId());
            if (!ch || !ch->hasUser(changeId))
                continue;                       

            ch->broadcast(msg, &user);          
        }
        
        else if (Utils::is_nickname(target))
        {
            User* dest = getUserByNick(target);
            if (dest) dest->addOutbox(msg);     
        }
        
    }
}


void Server::handleKick(User& user, const Parser& parser)
{
    
    const std::vector<std::string>& pr = parser.getParams();
    if (pr.size() < 2) {                                    
        user.addOutbox(":server 461 " + user.getNickName()
                       + " KICK :Not enough parameters\r\n");
        return;
    }
    const std::string& chanName   = pr[0];
    const std::string& victimNick = pr[1];
    std::string        comment    = (pr.size() > 2) ? pr[2] : user.getNickName();
    if (comment.empty() || comment[0] != ':') comment = ":" + comment;

    
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server 403 " + user.getNickName() + " "
                       + chanName + " :No such channel\r\n");
        return;
    }

    int changeId = ch->changeServerIdtoChannel(user.getId());
    
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(changeId);
    if (cit == ch->getChannelUsers().end() || cit->second->getAuth() != 7)
    {
        user.addOutbox(":server 482 " + user.getNickName() + " "
                       + chanName + " :You're not channel operator\r\n");
        return;
    }

    User* victim = NULL;
    for (TotalDatabase<ChannelData>::const_it  it = cit; it != ch->getChannelUsers().end(); it++){
        if (it->second->getWho()->getNickName() == victimNick){
            victim = it->second->getWho();
        }
    }

    
    if (!victim) {
        user.addOutbox(":server 401 " + user.getNickName() + " "
                       + victimNick + " :No such nick\r\n");
        return;
    }
    if (victim->getId() == user.getId()) {
        user.addOutbox(":server 482 " + user.getNickName() + " " +
                       chanName + " :You cannot kick yourself\r\n");
        return;
    }
    int changeVicId = ch->changeServerIdtoChannel(victim->getId());
    if (!ch->hasUser(changeVicId)) {
        user.addOutbox(":server 441 " + user.getNickName() + " "
                       + victimNick + " " + chanName +
                       " :They aren't on that channel\r\n");
        return;
    }

    
    std::string msg = ":" + user.getNickName() + "!" +
                      user.getUserName() + "@localhost KICK " +
                      chanName + " " + victimNick + " " + comment + "\r\n";

    
    ch->broadcast(msg, &user);                 

    
    ch->eraseUser(changeVicId);
    if (ch->getUserCount() == 0)
        ch->setInactive();
    else
        ch->ensureOneOp(); 
}



void Server::handleInvite(User& user, const Parser& parser)
{
    const std::vector<std::string>& pr = parser.getParams();
    if (pr.size() < 2) {                                    
        user.addOutbox(":server 461 " + user.getNickName() +
                       " INVITE :Not enough parameters\r\n");
        return;
    }

    const std::string& targetNick = pr[0];
    const std::string& chanName   = pr[1];

    
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server 403 " + user.getNickName() + " "
                       + chanName + " :No such channel\r\n");
        return;
    }

    
    int changeId = ch->changeServerIdtoChannel(user.getId());
    if (!ch->hasUserById(user.getId())) {                  
        user.addOutbox(":server 442 " + user.getNickName() + " "
                       + chanName + " :You're not on that channel\r\n");
        return;
    }

    
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(changeId);
    if (cit == ch->getChannelUsers().end() || cit->second->getAuth() < 7) {
        user.addOutbox(":server 482 " + user.getNickName() + " "
                       + chanName + " :You're not channel operator\r\n");
        return;
    }

    
    User* target = getUserByNick(targetNick);
    if (!target) {
        user.addOutbox(":server 401 " + user.getNickName() + " "
                       + targetNick + " :No such nick\r\n");
        return;
    }

    
    int changeTargetId = ch->changeServerIdtoChannel(target->getId());
    if (ch->hasUserById(target->getId())) {                
        user.addOutbox(":server 443 " + user.getNickName() + " "
                       + targetNick + " " + chanName +
                       " :is already on channel\r\n");
        return;
    }

    
    ch->addInvite(changeTargetId);                        

    
    std::string inviteMsg = ":" + user.fullPrefix() + " INVITE " +
                            targetNick + " :" + chanName + "\r\n";
    target->addOutbox(inviteMsg);

    
    user.addOutbox(":server 341 " + user.getNickName() + " "
                   + targetNick + " " + chanName + "\r\n");
}


void Server::handleTopic(User& user, const Parser& p)
{
    const std::vector<std::string>& pr = p.getParams();
    if (pr.empty()) {
        user.numeric(461, std::string("TOPIC :Not enough parameters"));
        return;
    }

    const std::string& chanName = pr[0];
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.numeric(403, chanName + " :No such channel");
        return;
    }

    int changeId = ch->changeServerIdtoChannel(user.getId());
    
    bool isOp = false;
    {
        TotalDatabase<ChannelData>::const_it cit =
            ch->getChannelUsers().getUserData(changeId);
        if (cit != ch->getChannelUsers().end() && cit->second->getAuth() >= 7)
            isOp = true;
    }

    
    if (pr.size() == 1) {
        const std::string& topic = ch->getTopic();
        if (topic.empty())
            user.numeric(331, chanName + " :No topic is set");
        else
            user.numeric(332, chanName + " :" + topic);
        return;
    }

    
    if (ch->isTopicOnly() && !isOp) {
        user.numeric(482, chanName + " :You're not channel operator");
        return;
    }

    std::string newTopic = pr[1];              
    if (newTopic == ":" || newTopic.empty()) newTopic.clear(); 
    ch->setTopic(newTopic);

    std::string msg = ":" + user.fullPrefix() + " TOPIC " +
                      chanName + " :" + newTopic + "\r\n";
    ch->broadcast(msg, NULL);
}

void Server::handleMode(User& user, const Parser& p)
{
    const std::vector<std::string>& pr = p.getParams();
    if (pr.size() < 2) {
        user.numeric(461, std::string("MODE :Not enough parameters"));
        return;
    }

    const std::string& chanName = pr[0];
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.numeric(403, chanName + " :No such channel"); return;
    }

    
    if (pr.size() == 1) {   
        std::string modes = "+";
        if (ch->isInviteOnly())
            modes += "i";
        if (ch->isTopicOnly())
            modes += "t";
        if (ch->getPwdSet())
            modes += "k";
        if (ch->getUserLimit())
            modes += "l";
        user.numeric(324, chanName + " " + modes);
        return;
    }

    
    int changeId = ch->changeServerIdtoChannel(user.getId());
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(changeId);
    if (cit == ch->getChannelUsers().end() || cit->second->getAuth() < 7) {
        user.numeric(482, chanName + " :You're not channel operator");
        return;
    }
    
    const std::string& modeStr = pr[1];
    size_t argIdx = 2;          
    char sign = 0;              
    bool ok = true;

    for (size_t i = 0; i < modeStr.size(); ++i) {
        char m = modeStr[i];
        if (m == '+' || m == '-') {
            sign = m; continue;
        }

        switch (m) {
        case 'i': ch->setInviteOnly(sign == '+'); break;
        case 't': ch->setTopicOnly (sign == '+'); break;
        case 'k':
            if (sign == '+') {
                if (ch->getPwdSet()){
                    user.numeric(467, chanName + " :Key already set"); ok=false; break;
                }
                if (argIdx >= pr.size() || !Utils::is_key(pr[argIdx])) {
                    user.numeric(461, std::string("MODE :Key param")); ok=false; break;
                }
                ch->setPwdset(true, pr[argIdx]);
            } else {
                ch->setPwdset(false);
            }
            break;
        case 'l':
        if (sign == '+') {
            if (argIdx >= pr.size()) {
                user.numeric(461, "MODE :Limit param");
                ok = false; break;
            }
            char* endp = 0;
            long v = std::strtol(pr[argIdx].c_str(), &endp, 10);

            
            if (*endp != '\0' || v <= 0 || v > MAX_USER_LIMIT) {
                user.numeric(461, "MODE :Bad limit value");
                ok = false; break;
            }
            ch->setUserLimit(static_cast<int>(v));
            ++argIdx;
            } else {
                ch->setUserLimit(0);
            }
            break;        
        case 'o':
            if (argIdx >= pr.size()) { user.numeric(461, std::string("MODE :o param")); ok=false; break; }
            applyOpFlag(ch, pr[argIdx++], sign == '+', user);
            break;
        default:
            user.numeric(472, std::string(1, m) + " :is unknown mode char");
            ok = false;
        }
    }

    if (ok) {
        std::string echo = ":" + user.fullPrefix() + " MODE " + chanName + " " + modeStr;
        
        for (size_t i = 2; i < pr.size(); ++i) echo += " " + pr[i];
        echo += "\r\n";

        ch->broadcast(echo, NULL);
    }
}


bool Server::handlePASS(User& u, const Parser& p){
    const std::vector<std::string>& params = p.getParams();
    if (params.size() != 1){
        u.addOutbox(":server 461 " + u.getNickName() + " PASS :Not enough parameters\r\n");
        return (false);
    }
    std::string pass = params[0];
    if (Utils::is_key(pass) == false){
        u.addOutbox(":server 464 " + u.getNickName() + " :Password incorrect\r\n");
        return (false);
    }
    if(this->_pwd.CheckPassword(pass) == false){
        u.addOutbox(":server 464 " + u.getNickName() + " :Password incorrect\r\n");
        return (false);
    }
    u.setNewby(32);
    return (true);
};


void Server::handleList(User& u){
    TotalDatabase<Channel>::const_it it = _channels.begin();
    for (; it != _channels.end(); it++){
        u.addOutbox(it->second->getChannelName());
        u.addOutbox(" acvive ");
        if (it->second->getIsActive() == true){
            u.addOutbox("true\n");
        }
        else{
            u.addOutbox("false\n");
        }
    }
};

void Server::handleShow(User& u, const Parser& p){
    
    const std::vector<std::string>& params = p.getParams();
    SharedPtr<Channel> channel;
    if (params.size() > 0 && Utils::is_channel(params[0])){
        for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it) {
            SharedPtr<Channel> temp = it->second;
            if (temp->getChannelName() == params[0]) {
                channel = it->second;
                break;
            }
        }
        if (channel.is_valid()   && channel->getIsActive()){
            TotalDatabase<ChannelData>::const_it it = channel->getChannelUsers().begin();
            for (; it != channel->getChannelUsers().end(); it++){
                u.addOutbox(it->second->getWho()->getNickName());
                u.addOutbox(" ");
                std::stringstream ss; ss << it->second->getAuth(); u.addOutbox(ss.str());
                u.addOutbox(" \n");
            }
        }
        else{
            u.addOutbox(":server 403 " + u.getNickName() + " " + params[0] + " :No such channel\r\n");
        }
        return ;
    }

    TotalDatabase<User>::const_it it = _users.begin();
    for (; it != _users.end(); it++){
        u.addOutbox(it->second->getNickName());
        u.addOutbox(" acvive ");
        if (it->second->getActive() == true){
            u.addOutbox("true\n");
        }
        else{
            u.addOutbox("false\n");
        }
    }
};


Channel* Server::getChannelByName(const std::string& name) {
    for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it)
        if (it->second->getChannelName() == name)
            return it->second.get();
    return NULL;
}

User* Server::getUserByNick(const std::string& nick) {
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it)
        if (it->second->getNickName() == nick)
            return it->second.get();
    return NULL;
}


int    Server::getSamefdUser(const int _pfdsFd){
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it){
        if (it->second->getFd() == _pfdsFd){
            return (it->second->getId());
        }
    }
    return (ERROR_ID);
}

void Server::applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             
                         User& src)
{
    User* tgt = getUserByNick(nick);
    if (!tgt || !ch->hasUserById(tgt->getId())) {
        src.numeric(441, nick + " " + ch->getChannelName() +
        " :They aren't on that channel");
        return;
    }
    int changeTgtId = ch->changeServerIdtoChannel(tgt->getId());
    SharedPtr<ChannelData> cd =
        ch->getChannelUsers().getUserData(changeTgtId)->second;
    cd->setAuth(give ? 7 : 0);

    
    std::string m = ":" + src.fullPrefix() + " MODE " +
                    ch->getChannelName() + (give?" +o ":" -o ") + nick + "\r\n";
    ch->broadcast(m, NULL);
}

void Server::stop() { live = false; }