#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Password.hpp"
#include "../include/SharedPtr.hpp"

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

    int changeId = ch->changeServerIdtoChannel(user.getId());
    /* 2. 채널 오퍼레이터 권한 (auth ≥ 7 이 op 라면 기존 로직 유지) */
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
    int changeVicId = ch->changeServerIdtoChannel(victim->getId());
    if (!ch->hasUser(changeVicId)) {
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
    ch->eraseUser(changeVicId);
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
    int changeId = ch->changeServerIdtoChannel(user.getId());
    if (!ch->hasUserById(user.getId())) {                  // ← 여기
        user.addOutbox(":server 442 " + user.getNickName() + " "
                       + chanName + " :You're not on that channel\r\n");
        return;
    }

    /* 3. (선택) op 권한 확인 */
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(changeId);
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
    int changeTargetId = ch->changeServerIdtoChannel(target->getId());
    if (ch->hasUserById(target->getId())) {                // ← 여기
        user.addOutbox(":server 443 " + user.getNickName() + " "
                       + targetNick + " " + chanName +
                       " :is already on channel\r\n");
        return;
    }

    /* 6. 초대장 기록(Invite-list) */
    ch->addInvite(changeTargetId);                        // Channel::addInvite()

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

    int changeId = ch->changeServerIdtoChannel(user.getId());
    /* OP 여부 캐시 */
    bool isOp = false;
    {
        TotalDatabase<ChannelData>::const_it cit =
            ch->getChannelUsers().getUserData(changeId);
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
    ch->broadcast(msg, NULL);
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
    int changeId = ch->changeServerIdtoChannel(user.getId());
    TotalDatabase<ChannelData>::const_it cit =
        ch->getChannelUsers().getUserData(changeId);
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
            if (*endp != '\0' || v <= 0 || v > MAX_USER_LIMIT /* 적당한 upper-bound */) {
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

        ch->broadcast(echo, NULL);
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
        if (channel->getIsActive()){
            TotalDatabase<ChannelData>::const_it it = channel->getChannelUsers().begin();
            for (; it != channel->getChannelUsers().end(); it++){
                u.addOutbox(it->second->getWho()->getNickName());
                u.addOutbox(" ");
                std::stringstream ss; ss << it->second->getAuth(); u.addOutbox(ss.str());
                u.addOutbox(" \n");
            }
        }
        else{
            u.addOutbox(":server ERROR Check Channel name\r\n");
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