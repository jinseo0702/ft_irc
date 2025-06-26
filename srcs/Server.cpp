#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>

#include <cerrno>


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
    User* rawUser = new User(777);                                                 //클래스, 함수. 함수 이름을 보면서 다음을 생각하기 어렵다

    _users.addUserWithId(rawUser);//Check

    SharedPtr<User> uPtr = _users.returnSecond(rawUser->getId());

    _lobby->addUser(uPtr);
    rawUser->addOutbox(":server NOTICE * :Welcome to Operator Ur in Lobby\r\n");
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

    fcntl(cfd, F_SETFL, O_NONBLOCK);

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
    u->setFd(-2);

    close(fd);
    _pfds.erase(_pfds.begin() + idx);
}


// 한 유저의 입력 읽기
void Server::_readLines(User& u, size_t idx){

    ssize_t n;
    char buf[2048] = {0,};
    if (u.getFd() == 0){
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
                return;                            // ◀︎ “조용히” 빠져나옴

            /* 그 밖의 진짜 오류는 연결 끊기 */
            _disconnectUser(idx);
            return;
        }
        if (n < 0)
        {
            /* 지금은 읽을 데이터가 없음 ⇒ 다음 poll 루프로 */
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;                            // ◀︎ “조용히” 빠져나옴

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
    for (int i = 2; i < this->_pfds.size(); i++){
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
        case PRIVMSG:   handlePrivMsg(user, parser); break;
        case NOTICE:    handleNotice(user, parser);  break;
        case KICK:      handleKick(user, parser);    break;
        case INVITE:    handleInvite(user, parser);  break;
        case TOPIC:     handleTopic(user, parser);   break;
        case MODE:      handleMode(user, parser);    break;
        case LIST:      handleList(user);    break;
        case SHOW:      handleShow(user);    break;
        default:
            user.addOutbox(":server ERROR unknown command\r\n");
            break;
    }
}

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
            SharedPtr<Channel> temp = it->second;
            if (temp->getChannelName() == channelName) {
                channel = it->second;
                break;
            }
        }
        if (!channel.is_valid()){//수정 할 부분
            int id = 0;
            id = this->_channels.addUserWithId(new Channel());
            channel = this->_channels.returnSecond(id);
            channel->setName(channelName);
            if (i < channelKeys.size() && Utils::is_key(channelKeys[i]))
                channel->setPwdset(true, channelKeys[i]);
                        // *** 채널 생성 시점 로그 ***
            std::cout << "[NEW CHANNEL] " << channelName << " created" << std::endl;
        }

        // 3. 패스워드(키) 검사
        if (channel->getPwdSet()) {
            std::string pass = (i < channelKeys.size()) ? channelKeys[i] : "";
        
            /* 1) key 가 없으면 바로 거절 */
            if (pass.empty()) {
                user.numeric(475, channelName + " :Cannot join channel (+k)"); // ERR_BADCHANNELKEY
                continue;
            }
            /* 2) 형식 검사 */
            if (!Utils::is_key(pass)) {
                user.numeric(467, channelName + " :Bad key format");           // RFC: 467
                continue;
            }
            /* 3) 일치 여부 */
            if (channel->checkPassword(pass) == false) {
                user.numeric(475, channelName + " :Wrong key");                // Same 475
                continue;
            }
        }
        if (channel->isInviteOnly() &&
            !channel->hasUserById(user.getId()) &&   // 아직 미참가
            !channel->isInvited(user.getId()))       // 초대 안 받음
        {
            user.numeric(473, channelName + " :Cannot join channel (+i)");
            continue;            // 이 채널은 거절 → 다음 채널로
        }
        channel->removeInvite(user.getId());
        // 4. 채널 가입 (중복 방지는 내부 addUser에서 처리)
        std::cout << "[JOIN TRY] " << user.getNickName() << " -> " << channelName << std::endl;
        SharedPtr<User> userPtr;
        userPtr = this->_users.returnSecond(user.getId());
        if (!userPtr.is_valid()){
            user.addOutbox(":server ERROR ERR_BADCHANMASK " + channelName + "\r\n");
            return ;
        }
        if (channel->hasUserById(user.getId())) {
            user.numeric(ERR_FATAL,
            channelName + " :is already on channel");
            continue;
        }
        channel->addUser(userPtr);
        channel->setIsActive();

        // 5. JOIN 메시지 브로드캐스트
        std::string joinMsg = ":" + user.getNickName() + "!" + user.getUserName() + "@localhost JOIN " + channelName + "\r\n";
        channel->broadcast(joinMsg, NULL);
    }
}



// NICK: 닉네임 설정
void Server::handleNick(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if(parser.parmsCnt() > 1){
        user.addOutbox(":server ERROR ERR_NEEDFEWPARAMS\r\n");
        return ;
    }
    if (params.empty() || params[0].empty()) {
        user.addOutbox(":server ERROR ERR_NONICKNAMEGIVEN\r\n");
        return;
    }

    std::string newNick = params[0];

    // 1. 유효한 닉네임 포맷인지 확인
    if (!Utils::is_nickname(newNick)) {
        user.addOutbox(":server ERROR ERR_ERRONEUSNICKNAME " + newNick + "\r\n");
        return;
    }

    // 2. 이미 사용 중인지 전체 유저 순회로 검사 (중복 불가)
    bool nickInUse = false;
    for (TotalDatabase<User>::const_it uit = _users.begin(); uit != _users.end(); ++uit) {
        if (uit->second->getNickName() == newNick) {
            nickInUse = true;
            break;
        }
    }
    if (nickInUse) {
        user.addOutbox(":server ERROR ERR_NICKNAMEINUSE " + newNick + "\r\n");
        return;
    }

    // 3. 기존 닉네임 저장
    std::string oldNick = user.getNickName();
    user.setNickName(newNick);
    user.setNewby(NICK);
    if (user.getActive() == false && user.getFd() > 3 && (user.getNewby() == 103)){
        user.setActive(true);
    }

    // 4. 이미 채널 참가중이면 채널 전체에 브로드캐스트 (ex: NICK oldNick -> newNick)
    // 모든 채널 순회
    for (TotalDatabase<Channel>::it chit = _channels.begin(); chit != _channels.end(); ++chit) {
        Channel* ch = chit->second.get();
        // 해당 채널에 이 유저가 있는지 검사
        for (Channel::UserIt uit = ch->userBegin(); uit != ch->userEnd(); ++uit) {
            SharedPtr<ChannelData> chData = uit->second;
            if (chData->getWho() == &user) {
                std::string notice;
                if (!oldNick.empty()) {
                    notice = ":" + oldNick + " NICK " + newNick + "\r\n";
                } else {
                    notice = ":" + newNick + " NICK " + newNick + "\r\n";
                }
                ch->broadcast(notice, NULL); // 전체에 알림
                break; // 한 채널에 한 번만!
            }
        }
    }

    // 5. 아직 채널에 참가한 적이 없다면 개인에게만 안내
    if (oldNick.empty()) {
        user.addOutbox(":" + newNick + " NICK " + newNick + "\r\n");
    }
}



// USER: 사용자 이름 설정
void Server::handleUser(User& u, const Parser& p)
{
    const std::vector<std::string>& params = p.getParams();

    // 1. 파라미터 수 확인 (username만 받도록 만들기)
    if (params.size() > 2) {
        u.addOutbox(":server ERROR ERR_NEEDFEWPARAMS USER\r\n");
        return;
    }

    // 2. 이미 설정된 경우 (중복 설정 방지)
    if (!u.getUserName().empty()) {
        u.addOutbox(":server ERROR ERR_ALREADYREGISTERED\r\n");
        return;
    }

    // 3. USER 정보 설정 (realname은 무시함)
    std::string username = params[0];
    u.setUserName(username);
    u.setNewby(USER);
    if (u.getActive() == false && u.getFd() > 3 && (u.getNewby() == 103)){
        u.setActive(true);
    }

    // 4. 성공 메시지 보내기 (선택 사항)
    std::string msg = ":server NOTICE * :Username set to " + username + "\r\n";
    u.addOutbox(msg);
}


// PART: 채널 나가기
void Server::handlePart(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if (params.empty()) {
        user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS PART\r\n");
        return;
    }

    // 채널 이름 목록 파싱 (콤마로 분리)
    std::istringstream iss(params[0]);
    std::string channelName;
    while (std::getline(iss, channelName, ',')) {
        Channel* ch = getChannelByName(channelName); // 존재하는 채널인지 확인
        if (!ch || !ch->getIsActive()) {
            user.addOutbox(":server ERROR ERR_NOSUCHCHANNEL " + channelName + "\r\n");
            continue;
        }

        // 채널에 유저가 존재하지 않으면 에러
        if (!ch->hasUser(user.getId())) {
            user.addOutbox(":server ERROR ERR_NOTONCHANNEL " + channelName + "\r\n");
            continue;
        }

        // PART 메시지 브로드캐스트 (자신 포함)
        std::string msg = ":" + user.getNickName() + "!" + user.getUserName()
                        + "@localhost PART " + channelName + "\r\n";
        ch->broadcast(msg, NULL); // from=NULL → 모두에게 보냄

        // 채널에서 유저 제거-------------------------------------------------------------------유저 제거하는게 맞나?
        ch->eraseUser(user.getId());

        // 유저에게도 직접 메시지 보냄 (대부분의 IRC 클라이언트는 이걸 기다림)
        user.addOutbox(msg);

        // 채널이 비면 비활성화 처리
        if (ch->getUserCount() == 0) {
            ch->setInactive(); // 방이 비었으면 종료
        }
        else
            ch->ensureOneOp();
    }
}


// QUIT: 서버 나가기
void Server::handleQuit(User& user, const Parser& parser)
{
    if (user.getFd() == 0){
        Parser p = Parser::parse("privmsg #lobby :Sever is Down bye bye");
        handlePrivMsg(user, p);
        this->live = false;
        return;
    }
    // 1. QUIT 메시지 파라미터 (종료 메시지)
    std::string quitMessage = "Client Quit";
    if (!parser.getParams().empty()) {
        quitMessage = parser.getParams()[0];
    }

    // 2. 전체 _channels 순회 후 유저 제거
    for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it) {
        SharedPtr<Channel> chPtr = it->second;
        if (!chPtr.is_valid()) 
            continue;
        Channel* ch = chPtr.get();

        // 활성화된 채널이고, 유저가 속해 있는 경우만 처리
        if (!ch->getIsActive() || !ch->hasUser(user.getId()))
            continue;

        // 3. QUIT 메시지 브로드캐스트
        std::string msg =
            ":" + user.getNickName() +
            "!" + user.getUserName() +
            "@localhost QUIT :" + quitMessage + "\r\n";
        ch->broadcast(msg, &user);

        // 4. 채널에서 유저 제거-------------------------------------------------------------------유저 제거하는게 맞나?
        ch->eraseUser(user.getId());
        if (ch->getUserCount() == 0) {
            ch->setInactive();
        }
        else
            ch->ensureOneOp(); 
    }

    // 5. 자기 자신에게도 QUIT 피드백 (nc 테스트용)
    user.addOutbox("ERROR :Closing Link: " + user.getNickName() +
                   " (" + quitMessage + ")\r\n");

    // 6. User 비활성화 표시 (poll 루프에서 소켓 close 조건으로 사용)
    for (size_t i = 1; i < _pfds.size(); ++i)
    {
    if (_pfds[i].fd == user.getFd()) {
        _disconnectUser(i);      // FD close + poll erase + users.erase()
        break;
    }
}
}



// PRIVMSG: 유저 또는 채널 대상 메시지
// === PRIVMSG ===
void Server::handlePrivMsg(User& user, const Parser& parser)
{
    const std::vector<std::string>& in = parser.getParams();

    /* 1. 최소 2개 확인 */
    if (in.size() < 2) {
        user.numeric(ERR_FATAL, "PRIVMSG :Not enough parameters");
        return;
    }

    /* 2. text 재결합 (trailing 포함) */
    std::string text = in[1];
    for (size_t i = 2; i < in.size(); ++i) {
        text += " " + in[i];
    }

    /* 3. 빈 텍스트 검사 */
    if (text.empty() || text == ":") {
        user.numeric(ERR_FATAL, ":No text to send");
        return;
    }
    if (text[0] != ':') text = ":" + text;

    /* 4. 대상 리스트 순회 */
    std::istringstream ts(in[0]);
    std::string target;
    while (std::getline(ts, target, ',')) {

        /* (a) 채널 대상 */
        if (Utils::is_channel(target)) {
            Channel* ch = getChannelByName(target);
            if (!ch) {
                user.numeric(ERR_NOSUCHCHANNEL, target + " :No such channel");
                continue;
            }
            if (!ch->hasUserGetServerId(user.getId())) {
                user.numeric(ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
                continue;
            }

            std::string msg = ":" + user.fullPrefix() +
                              " PRIVMSG " + target + " " + text + "\r\n";
            ch->broadcast(msg, &user);     // 자기 자신 제외

        /* (b) 닉네임 대상 */
        } 
        else if (Utils::is_nickname(target)) {
            User* dest = getUserByNick(target);
            if (!dest) {
                user.numeric(ERR_NOSUCHNICK, target + " :No such nick");
                continue;
            }

            std::string msg = ":" + user.fullPrefix() +
                              " PRIVMSG " + target + " " + text + "\r\n";
            dest->addOutbox(msg);

        /* (c) 그 외 → 잘못된 대상 */
        } 
        else {
            user.numeric(ERR_NOSUCHNICK, target + " :No such nick/channel");
        }
    }
}


// === NOTICE ===
void Server::handleNotice(User& user, const Parser& parser)
{
    const std::vector<std::string>& in = parser.getParams();
    if (in.size() < 2)
        return;                   // RFC: 오류 출력 X, 그냥 무시

    /* 1. 텍스트 trailing 복원 */
    std::string text = in[1];
    for (size_t i = 2; i < in.size(); ++i)
        text += " " + in[i];
    if (text.empty())
        return;                    // 빈 내용이면 무시
    if (text[0] != ':')
        text = ":" + text;       // 중복 콜론 방지

    /* 2. 대상 리스트 순회 */
    std::istringstream tss(in[0]);
    std::string target;

    while (std::getline(tss, target, ','))       // CSV
    {
        if (target.empty())
            continue;

        std::string msg = ":" + user.fullPrefix() +
                          " NOTICE " + target + " " + text + "\r\n";

        /* (a) 채널 대상 */
        if (Utils::is_channel(target))
        {
            Channel* ch = getChannelByName(target);
            if (!ch || !ch->hasUser(user.getId()))
                continue;                       // NOTICE: 오류 응답 없이 skip

            ch->broadcast(msg, &user);          // 자기 자신 제외
        }
        /* (b) 닉 대상 */
        else if (Utils::is_nickname(target))
        {
            User* dest = getUserByNick(target);
            if (dest) dest->addOutbox(msg);     // 없으면 skip
        }
        /* (c) 잘못된 토큰 → 아무 것도 하지 않음 (RFC 규정) */
    }
}

// === KICK === 6월 14일 readme 확인할것.
void Server::handleKick(User& user, const Parser& parser)
{
    /* 0. 파라미터 검사 ― KICK <channel> <nick> [ :comment ] */
    const std::vector<std::string>& pr = parser.getParams();
    if (pr.size() < 2) {                                    // 최소 2개
        user.addOutbox(":server 461 " + user.getNickName()
                       + " KICK :Not enough parameters\r\n");
        return;
    }
    const std::string& chanName   = pr[0];
    const std::string& victimNick = pr[1];
    std::string        comment    = (pr.size() > 2) ? pr[2] : user.getNickName();
    if (comment.empty() || comment[0] != ':') comment = ":" + comment;

    /* 1. 채널 존재 여부 */
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server 403 " + user.getNickName() + " "
                       + chanName + " :No such channel\r\n");
        return;
    }

    /* 2. 채널 오퍼레이터 권한 (auth ≥ 7 이 op 라면 기존 로직 유지) */
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(user.getId());
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

    /* 3. 대상 유저 확인 */
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
    if (!ch->hasUser(victim->getId())) {
        user.addOutbox(":server 441 " + user.getNickName() + " "
                       + victimNick + " " + chanName +
                       " :They aren't on that channel\r\n");
        return;
    }

    /* 4. KICK 메시지 작성 */
    std::string msg = ":" + user.getNickName() + "!" +
                      user.getUserName() + "@localhost KICK " +
                      chanName + " " + victimNick + " " + comment + "\r\n";

    /* 5. 브로드캐스트 ― 발신자·희생자 포함 채널 전체 */
    ch->broadcast(msg, &user);                 // skip 인자 없이 모두에게

    /* 6. 실제 제거 + 빈 채널 정리 */
    ch->eraseUser(victim->getId());
    if (ch->getUserCount() == 0)
        ch->setInactive();
    else
        ch->ensureOneOp(); 
}


// === INVITE === 6월 14일
void Server::handleInvite(User& user, const Parser& parser)
{
    const std::vector<std::string>& pr = parser.getParams();
    if (pr.size() < 2) {                                    // <nick> <channel>
        user.addOutbox(":server 461 " + user.getNickName() +
                       " INVITE :Not enough parameters\r\n");
        return;
    }

    const std::string& targetNick = pr[0];
    const std::string& chanName   = pr[1];

    /* 1. 채널 확인 */
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server 403 " + user.getNickName() + " "
                       + chanName + " :No such channel\r\n");
        return;
    }

    /* 2. 초대한 사람이 채널에 있는가? */
    if (!ch->hasUserById(user.getId())) {                  // ← 여기
        user.addOutbox(":server 442 " + user.getNickName() + " "
                       + chanName + " :You're not on that channel\r\n");
        return;
    }

    /* 3. (선택) op 권한 확인 */
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(user.getId());
    if (cit == ch->getChannelUsers().end() || cit->second->getAuth() < 7) {
        user.addOutbox(":server 482 " + user.getNickName() + " "
                       + chanName + " :You're not channel operator\r\n");
        return;
    }

    /* 4. 대상 유저 확인 */
    User* target = getUserByNick(targetNick);
    if (!target) {
        user.addOutbox(":server 401 " + user.getNickName() + " "
                       + targetNick + " :No such nick\r\n");
        return;
    }

    /* 5. 대상이 이미 채널에 있는가? (443) */
    if (ch->hasUserById(target->getId())) {                // ← 여기
        user.addOutbox(":server 443 " + user.getNickName() + " "
                       + targetNick + " " + chanName +
                       " :is already on channel\r\n");
        return;
    }

    /* 6. 초대장 기록(Invite-list) */
    ch->addInvite(target->getId());                        // Channel::addInvite()

    /* 8. INVITE 알림 → 대상 */
    std::string inviteMsg = ":" + user.fullPrefix() + " INVITE " +
                            targetNick + " :" + chanName + "\r\n";
    target->addOutbox(inviteMsg);

    /* 9. 341 RPL_INVITING → 발신자 */
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

    /* OP 여부 캐시 */
    bool isOp = false;
    {
        TotalDatabase<ChannelData>::const_it cit =
            ch->getChannelUsers().getUserData(user.getId());
        if (cit != ch->getChannelUsers().end() && cit->second->getAuth() >= 7)
            isOp = true;
    }

    /* 1) 조회 모드 */
    if (pr.size() == 1) {
        const std::string& topic = ch->getTopic();
        if (topic.empty())
            user.numeric(331, chanName + " :No topic is set");
        else
            user.numeric(332, chanName + " :" + topic);
        return;
    }

    /* 2) 변경 모드 */
    if (ch->isTopicOnly() && !isOp) {
        user.numeric(482, chanName + " :You're not channel operator");
        return;
    }

    std::string newTopic = pr[1];              // parser 가 ':' 포함 상태 유지
    if (newTopic == ":" || newTopic.empty()) newTopic.clear(); // 토픽 삭제
    ch->setTopic(newTopic);

    std::string msg = ":" + user.fullPrefix() + " TOPIC " +
                      chanName + " :" + newTopic + "\r\n";
    ch->broadcast(msg, nullptr);
}
//----------------MODE <#chan> <modestring> [argument]//
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

    /* 1. 조회 전용 (파라미터 1개) → 324 */
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

    /* 2. 수정: OP 권한 필수 */
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(user.getId());
    if (cit == ch->getChannelUsers().end() || cit->second->getAuth() < 7) {
        user.numeric(482, chanName + " :You're not channel operator");
        return;
    }
    //  플래그 파싱
    const std::string& modeStr = pr[1];
    size_t argIdx = 2;          // pr[argIdx]부터 추가 인자를 소비
    char sign = 0;              // 현재 부호(+ / -)
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
                ch->setPwdset(true, pr[argIdx++]);
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

            /* 숫자 변환 실패 또는 음수/0 은 거부 */
            if (*endp != '\0' || v <= 0 || v > 10000 /* 적당한 upper-bound */) {
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
        // 추가 인자들은 그대로 이어붙여 echo
        for (size_t i = 2; i < pr.size(); ++i) echo += " " + pr[i];
        echo += "\r\n";

        ch->broadcast(echo, nullptr);
    }
}

//서버에 들어오는 Newby가 비밀번호를 입력해야지 완벽하게 서버에 들어 올수 있습니다.
bool Server::handlePASS(User& u, const Parser& p){
    const std::vector<std::string>& params = p.getParams();
    if (params.size() != 1){
        u.addOutbox(":server ERROR Ceck PassWrod Params\r\n");
        return (false);
    }
    std::string pass = params[0];
    if (Utils::is_key(pass) == false){
        u.addOutbox(":server ERROR FATAL\r\n");
        return (false);
    }
    if(this->_pwd.CheckPassword(pass) == false){
        u.addOutbox(":server NOT correct\r\n");
        return (false);
    }
    u.setNewby(32);
    return (true);
};

//채널의 목록을 보여줍니다.
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

void Server::handleShow(User& u){
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

//it return Id 를 반환 fd값이 존재하지 않는다면 -999 값을반환 합니다. 이경우 치명적인 error 입니다.
int    Server::getSamefdUser(const int _pfdsFd){
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it){
        if (it->second->getFd() == _pfdsFd){
            return (it->second->getId());
        }
    }
    return (-999);
}

void Server::applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             // true = +o, false = -o
                         User& src)
{
    User* tgt = getUserByNick(nick);
    if (!tgt || !ch->hasUserById(tgt->getId())) {
        src.numeric(441, nick + " " + ch->getChannelName() +
                          " :They aren't on that channel");
        return;
    }
    SharedPtr<ChannelData> cd =
        ch->getChannelUsers().getUserData(tgt->getId())->second;
    cd->setAuth(give ? 7 : 0);

    /* MODE echo (op 변경은 곧바로 채널에도 전파) */
    std::string m = ":" + src.fullPrefix() + " MODE " +
                    ch->getChannelName() + (give?" +o ":" -o ") + nick + "\r\n";
    ch->broadcast(m, nullptr);
}

void Server::stop() { live = false; }