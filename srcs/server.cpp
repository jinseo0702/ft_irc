#include "../include/server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
// static var
bool Server::readflag = false;

// 생성자
Server::Server(int port, const std::string& password)
    : _listenFd(-1), _password(password), _lobby(NULL)
{
    _setupSocket(port);

    // 0번 loby 채널 생성 및 등록
    Channel* lobby = new Channel();
    lobby->setId(0);
    lobby->setName("#lobby");
    this->_channels.addUserWithId(lobby);
    this->_lobby = lobby;
    std::cout << "Listening on port " << port << " (password: " << password << "), #lobby created" << std::endl;
}

// 소켓 설정
void Server::_setupSocket(int port)
{
    this->_listenFd = socket(AF_INET, SOCK_STREAM, 0);
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
    if (listen(this->_listenFd, 20) < 0)
        throw std::runtime_error("listen error");

    fcntl(this->_listenFd, F_SETFL, O_NONBLOCK);
    struct pollfd pfd;
    pfd.fd = this->_listenFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_pfds.push_back(pfd);
}

// 메인 루프
void Server::run()
{
    while (true)
    {
        if (poll(&this->_pfds[0], this->_pfds.size(), -1) < 0)
            throw std::runtime_error("poll error");

        // (A) 새 연결
        if (this->_pfds[0].revents & POLLIN)
            _acceptClient();

        // (B) 각 유저별 처리
        size_t i = 1;
        while (i < this->_pfds.size())
        {
            User& user = *(_users.getUserData(i-1)->second); // userId = i-1 로 예시
            if (this->_pfds[i].revents & POLLIN)
                _readLines(user, i);

            if (this->readflag)
                this->_pfds[i].events |= POLLOUT;

            if (i < this->_pfds.size() && (this->_pfds[i].revents & POLLOUT))
                _flushOut(user, i);

            ++i;
        }
        this->readflag = false;
    }
}

// 새 클라이언트 수락
void Server::_acceptClient()
{
    int cfd = accept(this->_listenFd, 0, 0);
    if (cfd < 0)
        return;
    fcntl(cfd, F_SETFL, O_NONBLOCK);

    struct pollfd pfd;
    pfd.fd = cfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_pfds.push_back(pfd);

    // 유저 등록 (id 자동 증가)
    User* newUser = new User(cfd);
    this->_users.addUserWithId(newUser);

    // 로비 채널에 유저 추가
    this->_lobby->addUser(SharedPtr<User>(newUser));

    // 환영 메시지
    newUser->addOutbox(":server NOTICE * :Welcome to #lobby\r\n");
    this->_pfds.back().events |= POLLOUT;
}

// 한 유저의 입력 읽기
void Server::_readLines(User& u, size_t idx)
{
    char buf[512];
    ssize_t n = recv(u.getFd(), buf, sizeof(buf)-1, 0);
    if (n <= 0)
    {
        _disconnectUser(idx);
        return;
    }
    buf[n] = '\0';
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
    this->readflag = true;
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

void Server::_disconnectUser(size_t idx)
{
    close(this->_pfds[idx].fd);
    this->_pfds.erase(this->_pfds.begin() + idx);
    // TODO: 유저/채널 관리에서 삭제
}

// fd → string
std::string Server::_fdToStr(int fd) const {
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}

// Command Dispatcher (Rulehandle 활용)
void Server::_dispatch(User& user, const Parser& parser)
{
    Rulehandle::Mypair cmdinfo = Rulehandle::checkCommand(parser);
    user_role cmd = cmdinfo.second;

    if (Rulehandle::isError(cmd)) {
        user.addOutbox(":server ERROR " + cmdinfo.first + "\r\n");
        return;
    }

    switch(cmd) {
        case JOIN:      handleJoin(user, parser);    break;
        case NICK:      handleNick(user, parser);    break;
        case USER:      handleUser(user, parser);    break;
        case PART:      handlePart(user, parser);    break;
        case QUIT:      handleQuit(user, parser);    break;
        case PRIVMSG:   handlePrivMsg(user, parser); break;
        case NOTICE:    handleNotice(user, parser);  break;
        case KICK:      handleKick(user, parser);    break;
        case INVITE:    handleInvite(user, parser);  break;
        case TOPIC:     handleTopic(user, parser);   break;
        case MODE:      handleMode(user, parser);    break;
        default:
            user.addOutbox(":server ERROR unknown command\r\n");
            break;
    }
}

// (아래 핸들러 함수들은 골격/샘플만)
// JOIN: 채널 생성 또는 참가

void Server::handleJoin(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if (params.empty()) {
        user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS\r\n");
        return;
    }

    // 1. 채널/키 분리
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
            user.addOutbox(":server ERROR ERR_BADCHANMASK " + channelName + "\r\n");
            continue;
        }
        SharedPtr<Channel> channel;
        for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it) {
            if (it->second->getChannelName() == channelName) {
                channel = it->second;
                break;
            }
        }
        if (!channel.is_valid()) {
            Channel* newChan = new Channel();
            newChan->setName(channelName);
            _channels.addUserWithId(newChan);
            channel = SharedPtr<Channel>(newChan);
                        // *** 채널 생성 시점 로그 ***
            std::cout << "[NEW CHANNEL] " << channelName << " created" << std::endl;
        }

        // 3. 패스워드(키) 검사
        if (channel->getPwdSet()) {
            std::string pass = (i < channelKeys.size()) ? channelKeys[i] : "";
            if (!Utils::is_key(pass)) {
                user.addOutbox(":server ERROR ERR_BADCHANNELKEY " + channelName + "\r\n");
                continue;
            }
            if (pass != "" && channel->getPwdSet() != atoi(pass.c_str())) { //채널에 겟 패스워드 
                user.addOutbox(":server ERROR ERR_BADCHANNELKEY " + channelName + "\r\n");
                continue;
            }
        }

        // 4. 채널 가입 (중복 방지는 내부 addUser에서 처리)
        std::cout << "[JOIN TRY] " << user.getNickName() << " -> " << channelName << std::endl;
        channel->addUser(SharedPtr<User>(&user));

        channel->setIsActve();

        // 5. JOIN 메시지 브로드캐스트
        std::string joinMsg = ":" + user.getNickName() + "!" + user.getUserName() + "@localhost JOIN " + channelName + "\r\n";
        channel->broadcast(joinMsg, NULL);
    }
}


// NICK: 닉네임 설정
void Server::handleNick(User& u, const Parser& p) {
    // TODO: 중복 닉네임 검사, 변경
}

// USER: 사용자 이름 설정
void Server::handleUser(User& u, const Parser& p) {
    // TODO: 사용자 정보 등록
}

// PART: 채널 나가기
void Server::handlePart(User& u, const Parser& p) {
    // TODO: 채널 리스트에서 제거
}

// QUIT: 서버 나가기
void Server::handleQuit(User& u, const Parser& p) {
    // TODO: 전체 채널에서 제거, 연결 종료
}

// PRIVMSG: 쪽지/채널 메시지
void Server::handlePrivMsg(User& u, const Parser& p) {
    // TODO: 채널 혹은 유저에게 메시지 브로드캐스트
}

// NOTICE: 쪽지/공지
void Server::handleNotice(User& u, const Parser& p) {
    // TODO
}

void Server::handleKick(User& u, const Parser& p) { /* TODO */ }
void Server::handleInvite(User& u, const Parser& p) { /* TODO */ }
void Server::handleTopic(User& u, const Parser& p) { /* TODO */ }
void Server::handleMode(User& u, const Parser& p) { /* TODO */ }

// -- 여기서 아래로는 유틸 함수 샘플 (실제 구현 필요) --
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
