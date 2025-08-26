#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Password.hpp"
#include "../include/SharedPtr.hpp"


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
        int changeId = channel->changeServerIdtoChannel(user.getId());
        if (channel->isInviteOnly() &&
            !channel->hasUserById(user.getId()) &&   // 아직 미참가
            !channel->isInvited(changeId))       // 초대 안 받음
        {
            user.numeric(473, channelName + " :Cannot join channel (+i)");
            continue;            // 이 채널은 거절 → 다음 채널로
        }
        channel->removeInvite(changeId);
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
        int changeId = ch->changeServerIdtoChannel(user.getId());
        // 채널에 유저가 존재하지 않으면 에러
        if (!ch->hasUser(changeId)) {
            user.addOutbox(":server ERROR ERR_NOTONCHANNEL " + channelName + "\r\n");
            continue;
        }

        // PART 메시지 브로드캐스트 (자신 포함)
        std::string msg = ":" + user.getNickName() + "!" + user.getUserName()
                        + "@localhost PART " + channelName + "\r\n";
        ch->broadcast(msg, NULL); // from=NULL → 모두에게 보냄

        // 채널에서 유저 제거-------------------------------------------------------------------유저 제거하는게 맞나?
        ch->eraseUser(changeId);

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
    if (user.getFd() == STDIN_FD){
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

        int changeId = ch->changeServerIdtoChannel(user.getId());
        // 활성화된 채널이고, 유저가 속해 있는 경우만 처리
        if (!ch->getIsActive() || !ch->hasUser(changeId))
            continue;

        // 3. QUIT 메시지 브로드캐스트
        std::string msg =
            ":" + user.getNickName() +
            "!" + user.getUserName() +
            "@localhost QUIT :" + quitMessage + "\r\n";
        ch->broadcast(msg, &user);

        // 4. 채널에서 유저 제거-------------------------------------------------------------------유저 제거하는게 맞나?
        ch->eraseUser(changeId);
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
            int changeId = ch->changeServerIdtoChannel(user.getId());
            if (!ch || !ch->hasUser(changeId))
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