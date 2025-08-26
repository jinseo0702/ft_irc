#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Password.hpp"
#include "../include/SharedPtr.hpp"

// 생성자
Server::Server(int port, std::string& password)
    : _listenFd(-1)
{
    _setupSocket(port);

    /* 0번 #lobby 채널 생성 ----------------------------------- */
    SharedPtr<Channel> lobby(new Channel());   // 스마트포인터 한 줄

    /* TotalDatabase 에 등록 (addUserWithId 가 SharedPtr 인수여야 함) */
    this->_channels.addUserWithId(lobby);

    /* 멤버에 보관 */
    this->_lobby = lobby;

    struct pollfd pfd = { 
        STDIN_FILENO, POLLIN, 0 
    };
 
    _pfds.push_back(pfd);
    User* rawUser = new User(SUPER_USER_FD);                                                 //클래스, 함수. 함수 이름을 보면서 다음을 생각하기 어렵다

    _users.addUserWithId(rawUser);//Check

    SharedPtr<User> uPtr = _users.returnSecond(rawUser->getId());

    _lobby->addUser(uPtr);
    rawUser->addOutbox(":server NOTICE * :Welcome to Operator Ur in Lobby\r\n");
    //bot 세팅하기
    _initializeBot();
    /* 패스워드 세팅 ----------------------------------------- */
    _pwd.setisPasswordSet(true);
    _pwd.setPwd(password);

    std::cout << "Listening on port " << port
              << " (password: " << password << "), #lobby created\n";
    this->live = true;
}


// 소켓 설정
void Server::_setupSocket(int port)
{
    this->_listenFd = socket(AF_INET, SOCK_STREAM | 2048, 0);
    if (this->_listenFd < 0)
        throw std::runtime_error("socket error");
    // 1. 먼저 일반적인 '블로킹' 소켓을 생성합니다.
    //this->_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    //if (this->_listenFd < 0)
    //    throw std::runtime_error("socket error");

    // 2. fcntl() 함수를 사용해서 소켓을 '논-블로킹' 모드로 직접 변경합니다.
    //if (fcntl(this->_listenFd, F_SETFL, O_NONBLOCK) < 0)
    //    throw std::runtime_error("fcntl error"); // fcntl 실패에 대한 예외 처리
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

// 메인 루프
void Server::run()
{
    while (true)
    {
        if (Sig::stopRequested()) {
            stop();                 // _live = false;
            break;                  // poll() 안 깨우고 flag만 체크해도 됨
        }
        if (poll(&this->_pfds[0], this->_pfds.size(), -1) < 0){
            if (errno == EINTR)      // 신호로 깨진 경우
                continue;            // 다시 루프 → flag 검사
            throw std::runtime_error("poll error");
        }   
        if (this->live == false){
            break;
        }
        // (A) 새 연결
        if (this->_pfds[0].revents & POLLIN){
            _acceptClient();
        }

        // (B) 각 유저별 처리
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
        // (C) DCC 전송 처리
        _dccManager.processDCCTransfers();    
        // (D) 봇 메시지 처리
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
    /*
    for (int k = 2; k < _pfds.size(); k++){
        _disconnectUser(k);
    }
    */
    for (int k = static_cast<int>(_pfds.size()) - 1; k >= 2; --k)
        _disconnectUser(k);
    if (_listenFd >= 0)
        close(_listenFd);   // ★ 리스닝 FD 반납
    return ;
}

void Server::_acceptClient()
{
    /* 1. 소켓 수락 + 논블로킹 ------------------------------------------ */
    int cfd = accept(_listenFd, 0, 0);
    if (cfd < 0)        // 에러면 무시
        return;

    struct pollfd pfd = { cfd, POLLIN, 0 };
    _pfds.push_back(pfd);

    /* 2. User 객체를 한 번에 SharedPtr 로 ------------------------------ */
    SharedPtr<User> u(new User(cfd));     // ★ new → 즉시 스마트포인터로 감쌈

    /* 3. 전역 유저 DB에 등록  (TotalDatabase::add 는 SharedPtr 인수) */
    _users.addUserWithId(u);                        // 내부에서 id 부여 + ref-count++  

    /* 4. #lobby 채널에도 같은 포인터 공유 ------------------------------ */
    _lobby->addUser(u);                   // ref-count 또 +1 (공유만 할 뿐)

    /* 5. 환영 메시지 큐에 넣고, 곧바로 송신 가능하도록 POLLOUT 세팅 */
    u->addOutbox(":server NOTICE * :Welcome to #lobby\r\n");
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


// 한 유저의 입력 읽기
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
            /* 지금은 읽을 데이터가 없음 ⇒ 다음 poll 루프로 */
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;                            // ◀︎ "조용히" 빠져나옴

            /* 그 밖의 진짜 오류는 연결 끊기 */
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

// fd → string
std::string Server::_fdToStr(int fd) const {
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}
//여기까지가 코어 밑으로 command

// Command Dispatcher (Rulehandle 활용)
void Server::_dispatch(User& user, const Parser& parser)
{
    Rulehandle::Mypair cmdinfo = Rulehandle::checkCommand(parser);
    user_role cmd = cmdinfo.second;

    if (Rulehandle::isError(cmd)) {
        user.addOutbox(":server ERROR " + cmdinfo.first + "\r\n");
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
            _handleBotCommands(user, parser);
            break;
        case NOTICE:    handleNotice(user, parser);  break;
        case KICK:      handleKick(user, parser);    break;
        case INVITE:    handleInvite(user, parser);  break;
        case TOPIC:     handleTopic(user, parser);   break;
        case MODE:      handleMode(user, parser);    break;
        case LIST:      handleList(user);    break;
        case SHOW:      handleShow(user, parser);    break;
        // DCC 명령어들 추가
        case DCC_SEND:  handleDCCSend(user, parser); break;
        case DCC_ACCEPT: handleDCCAccept(user, parser); break;
        case DCC_RESUME: handleDCCResume(user, parser); break;
        case DCC_REJECT: handleDCCReject(user, parser); break;
        default:
            user.addOutbox(":server ERROR unknown command\r\n");
            break;
    }
}
