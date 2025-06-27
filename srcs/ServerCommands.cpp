#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <sstream>
#include <cstring>

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
            // 봇 명령어 처리 추가
            _handleBotCommands(user, parser);
            break;
        case NOTICE:    handleNotice(user, parser);  break;
        case KICK:      handleKick(user, parser);    break;
        case INVITE:    handleInvite(user, parser);  break;
        case TOPIC:     handleTopic(user, parser);   break;
        case MODE:      handleMode(user, parser);    break;
        case LIST:      handleList(user);    break;
        case SHOW:      handleShow(user);    break;
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

        // 채널이 비어있으면 채널 삭제
        if (ch->getUserCount() == 0) {
            ch->setInactive();
            std::cout << "[CHANNEL EMPTY] " << channelName << " is now empty" << std::endl;
        }
    }
}

// PASS: 패스워드 설정
bool Server::handlePASS(User& u, const Parser& p)
{
    const std::vector<std::string>& params = p.getParams();
    if (params.empty()) {
        u.addOutbox(":server ERROR ERR_NEEDMOREPARAMS PASS\r\n");
        return false;
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
}

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
}

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
}

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
    ch->broadcast(m, NULL);
}

void Server::handleKick(User& user, const Parser& parser) {}
void Server::handleMode(User& user, const Parser& parser) {}
void Server::handleQuit(User& user, const Parser& parser) {}
void Server::handleTopic(User& user, const Parser& parser) {}
void Server::handleInvite(User& user, const Parser& parser) {}
void Server::handleNotice(User& user, const Parser& parser) {}
void Server::handlePrivMsg(User& user, const Parser& parser) {} 