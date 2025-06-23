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


// 생성자
Server::Server(int port, std::string& password)
    : _listenFd(-1)
{
    _setupSocket(port);

    /* 0번 #lobby 채널 생성 ----------------------------------- */
    SharedPtr<Channel> lobby(new Channel());   // 스마트포인터 한 줄
    // lobby->setId(0);
    // lobby->setName("#lobby");

    /* TotalDatabase 에 등록 (addUserWithId 가 SharedPtr 인수여야 함) */
    this->_channels.addUserWithId(lobby);

    /* 멤버에 보관 */
        this->_lobby = lobby;
    User* rawUser = new User();                                                   //클래스, 함수. 함수 이름을 보면서 다음을 생각하기 어렵다

    _users.addUserWithId(rawUser);//Check

    SharedPtr<User> uPtr = _users.returnSecond(rawUser->getId());

    _lobby->addUser(uPtr);
    rawUser->addOutbox(":server NOTICE * :Welcome to Operator Ur in Lobby\r\n");
    /* 패스워드 세팅 ----------------------------------------- */
    _pwd.setisPasswordSet(true);
    _pwd.setPwd(password);

    std::cout << "Listening on port " << port
              << " (password: " << password << "), #lobby created\n";
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
        if (poll(&this->_pfds[0], this->_pfds.size(), -1) < 0){
            throw std::runtime_error("poll error");
        }

        // (A) 새 연결
        if (this->_pfds[0].revents & POLLIN){
            _acceptClient();
        }

        // (B) 각 유저별 처리
        size_t i = 0;
        while (++i < this->_pfds.size()){
            // User& user = *(_users.getUserData(i-1)->second); // userId = i-1 로 예시
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
    std::cout << "Listening #lobby created" << std::endl;
}

// 새 클라이언트 수락
void Server::_acceptClient(){
    int cfd = accept(_listenFd, 0, 0);
    if (cfd < 0){
        return;
    }
    fcntl(cfd, F_SETFL, O_NONBLOCK);

    struct pollfd pfd = { 
        cfd,
        POLLIN,
        0 
    };

    _pfds.push_back(pfd);

    User* rawUser = new User(cfd);                                                   //클래스, 함수. 함수 이름을 보면서 다음을 생각하기 어렵다

    _users.addUserWithId(rawUser);//Check

    SharedPtr<User> uPtr = _users.returnSecond(rawUser->getId());

    _lobby->addUser(uPtr);

    rawUser->addOutbox(":server NOTICE * :Welcome to #lobby\r\n");
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
void Server::_readLines(User& u, size_t idx)
{
    char buf[512] = {0,};
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
    for (int i = 1; i < this->_pfds.size(); ++i){
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
        SharedPtr<User> userPtr;
        userPtr = this->_users.returnSecond(user.getId());
        if (!userPtr.is_valid()){
            user.addOutbox(":server ERROR ERR_BADCHANMASK " + channelName + "\r\n");
            return ;
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
    }
}


// QUIT: 서버 나가기
void Server::handleQuit(User& user, const Parser& parser)
{
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
    }

    // 5. 자기 자신에게도 QUIT 피드백 (nc 테스트용)
    user.addOutbox("ERROR :Closing Link: " + user.getNickName() +
                   " (" + quitMessage + ")\r\n");

    // 6. User 비활성화 표시 (poll 루프에서 소켓 close 조건으로 사용)
    user.setActive(false);
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
            if (!ch->hasUser(user.getId())) {
                user.numeric(ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
                continue;
            }

            std::string msg = ":" + user.fullPrefix() +
                              " PRIVMSG " + target + " " + text + "\r\n";
            ch->broadcast(msg, &user);     // 자기 자신 제외

        /* (b) 닉네임 대상 */
        } else if (Utils::is_nickname(target)) {
            User* dest = getUserByNick(target);
            if (!dest) {
                user.numeric(ERR_NOSUCHNICK, target + " :No such nick");
                continue;
            }

            std::string msg = ":" + user.fullPrefix() +
                              " PRIVMSG " + target + " " + text + "\r\n";
            dest->addOutbox(msg);

        /* (c) 그 외 → 잘못된 대상 */
        } else {
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
    if (text.empty()) return;                    // 빈 내용이면 무시
    if (text[0] != ':') text = ":" + text;       // 중복 콜론 방지

    /* 2. 대상 리스트 순회 */
    std::istringstream tss(in[0]);
    std::string target;

    while (std::getline(tss, target, ','))       // CSV
    {
        if (target.empty()) continue;

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
    if (ch->getUserCount() == 0) ch->setInactive();
}


// === INVITE === 6월 14일
void Server::handleInvite(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    if (params.size() < 2) {
        user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS INVITE\r\n");
        return;
    }

    std::string targetNick = params[0];
    std::string chanName   = params[1];

    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server ERROR ERR_NOSUCHCHANNEL " + chanName + "\r\n");
        return;
    }

    if (!ch->hasUser(user.getId())) {
        user.addOutbox(":server ERROR ERR_NOTONCHANNEL " + chanName + "\r\n");
        return;
    }

    TotalDatabase<ChannelData>::const_it cit2 =
        ch->getChannelUsers().getUserData(user.getId());
    if (cit2 == ch->getChannelUsers().end() ||
        cit2->second->getAuth() != 7)
    {
        user.addOutbox(":server ERROR ERR_CHANOPRIVSNEEDED " + chanName + "\r\n");
        return;
    }

    User* target = getUserByNick(targetNick);
    if (!target) {
        user.addOutbox(":server ERROR ERR_NOSUCHNICK " + targetNick + "\r\n");
        return;
    }

    std::string msg = ":"
        + user.getNickName() + "!"
        + user.getUserName() + "@localhost INVITE "
        + targetNick + " :" + chanName + "\r\n";

    target->addOutbox(msg);

    // RPL_INVITING (341) 응답
    std::string rep = ":server 341 "
        + user.getNickName() + " "
        + targetNick + " "
        + chanName + "\r\n";
    user.addOutbox(rep);
}


void Server::handleTopic(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    // 파라미터 1개(채널) 아니면 에러
    if (params.empty()) {
        user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS TOPIC\r\n");
        return;
    }

    std::string chanName = params[0];
    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server ERROR ERR_NOSUCHCHANNEL " + chanName + "\r\n");
        return;
    }

    // 아무런 토픽 인자가 없으면 현재 토픽 보내기
    if (params.size() == 1) {
        const std::string& topic = ch->getTopic();
        if (topic.empty()) {
            user.addOutbox(":server 331 " + user.getNickName()
                + " " + chanName
                + " :No topic is set\r\n");
        } else {
            user.addOutbox(":server 332 " + user.getNickName()
                + " " + chanName
                + " :" + topic + "\r\n");
        }
        return;
    }

    // 토픽 변경 권한 검사: 채널 오퍼레이터만
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(user.getId());
    if (cit == ch->getChannelUsers().end() ||
        cit->second->getAuth() < 7)
    {
        user.addOutbox(":server ERROR ERR_CHANOPRIVSNEEDED " + chanName + "\r\n");
        return;
    }

    // 새 토픽 조합 (params[1] 이후 모두 1개 문자열로 parser가 만들어 주었다고 가정)
    std::string newTopic = params[1];
    ch->setTopic(newTopic);

    // 브로드캐스트 : :nick!user@host TOPIC #chan :newTopic
    std::string msg = ":" + user.getNickName()
                    + "!" + user.getUserName()
                    + "@localhost TOPIC "
                    + chanName
                    + " :" + newTopic
                    + "\r\n";
    ch->broadcast(msg, &user);
}

void Server::handleMode(User& user, const Parser& parser)
{
    const std::vector<std::string>& params = parser.getParams();
    // 최소 2개: MODE <#chan> <mode> [<arg>]
    if (params.size() < 2) {
        user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS MODE\r\n");
        return;
    }

    std::string chanName = params[0];
    std::string modeStr  = params[1];
    std::string arg      = (params.size() >= 3 ? params[2] : "");

    Channel* ch = getChannelByName(chanName);
    if (!ch) {
        user.addOutbox(":server ERROR ERR_NOSUCHCHANNEL " + chanName + "\r\n");
        return;
    }

    // 오퍼레이터 권한 체크
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(user.getId());
    if (cit == ch->getChannelUsers().end() ||
        cit->second->getAuth() < 7)
    {
        user.addOutbox(":server ERROR ERR_CHANOPRIVSNEEDED " + chanName + "\r\n");
        return;
    }

    // modeStr 예: "+i", "-k", "+l", "-t"
    bool success = true;
    std::string reply = ":" + user.getNickName() + "!" + user.getUserName()
                      + "@localhost MODE " + chanName + " " + modeStr;

    if (modeStr == "+i") {
        ch->setInviteOnly(true);
    }
    else if (modeStr == "-i") {
        ch->setInviteOnly(false);
    }
    else if (modeStr == "+t") {
        ch->setTopicOnly(true);
    }
    else if (modeStr == "-t") {
        ch->setTopicOnly(false);
    }
    else if (modeStr == "+k") {
        // 키 설정: arg 가 숫자 형태라고 가정
        if (arg.empty() || !Utils::is_key(arg)) {
            success = false;
            user.addOutbox(":server ERROR ERR_KEYSET " + chanName + "\r\n");
        } else {
            ch->setPwdset(true, arg);
            reply += " :" + arg;
        }
    }
    else if (modeStr == "-k") {
        ch->setPwdset(false);
    }
    else if (modeStr == "+l") {
        if (arg.empty()) {
            success = false;
            user.addOutbox(":server ERROR ERR_NEEDMOREPARAMS MODE\r\n");
        } else {
            int limit = atoi(arg.c_str());
            ch->setUserLimit(limit);
            reply += " :" + arg;
        }
    }
    else if (modeStr == "-l") {
        ch->setUserLimit(0);
    }
    else {
        success = false;
        user.addOutbox(":server ERROR ERR_NOSUCHMODEOPTION " + modeStr + "\r\n");
    }

    if (success) {
        reply += "\r\n";
        // 채널 내 전체 브로드캐스트
        ch->broadcast(reply, &user);
        // 호출자에도 전송
        user.addOutbox(reply);
    }
};

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
