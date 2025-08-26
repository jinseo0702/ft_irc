#include "../include/Channel.hpp"
#include "../include/User.hpp"

Channel::Channel()
  : TotalChannelID(-1),
    isActive(false),
    inviteOnly(false),
    topicOnly(false),
    userLimit(0)
{}

Channel::~Channel(){

};

//채널자신의 아이디
int Channel::getTotalChannelID() const{
    return (this->TotalChannelID);
};

//채널자신의 이름
std::string Channel::getChannelName() const{
    return (this->ChannelName);
};

//각 채널 안 User id 찾기 / <ChannelData> 에 할당된 user의 아이디 서버 (id x)
TotalDatabase<ChannelData>::const_it const Channel::getChannelUser(int ChannelUserID) const{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    return (it);
};

//채널 비밀번호 지정
bool Channel::getPwdSet() const{
    return (this->pwd.getIsPasswordSet());
};

//채널의 활성화 보기
bool Channel::getIsActive() const{
    return (this->isActive);
};

//채널이름 지정
void Channel::setName(const std::string &obj){
    this->ChannelName = obj;
};

//각 채널 유저 추가 
void Channel::addUser(SharedPtr<User> newUser){
    int before = this->ChannelUser.sizeData();
    this->ChannelUser.addUserWithId(new ChannelData(newUser));
    if (this->ChannelUser.getUserData(0) != this->ChannelUser.end()){
        this->ChannelUser.getUserData(0)->second->setAuth(7);}
    int after = this->ChannelUser.sizeData();
    std::cout << "[ADD USER] try: " 
              << (newUser.is_valid() ? newUser->getNickName() : "NULL") 
              << " (" << (newUser.is_valid() ? newUser->getFd() : -1) << ") "
              << " to channel " << this->ChannelName 
              << ", user count: " << after
              << " (before: " << before << ", delta: " << (after-before) << ")\n";

    // 유저 목록 전체 출력
    std::cout << "[USER LIST for " << this->ChannelName << "]: ";
    for (TotalDatabase<ChannelData>::const_it uit = this->ChannelUser.begin(); uit != this->ChannelUser.end(); ++uit) {
        SharedPtr<ChannelData> chd = uit->second;

        if (chd.is_valid() && chd->getWho())
            std::cout << chd->getWho()->getNickName() << "(" << chd->getWho()->getFd() << ") ";
        else
            std::cout << "[?] ";
    }
    std::cout << std::endl;
}
//진짜 말그대로 채널 ID 지정
void Channel::setId(int id){
    if (id == 0){
        this->ChannelName = "#lobby";
    }
    this->TotalChannelID = id;
};

//UserData와 호환이 되도록 만들어야 겠습니다. UserData에서 관리자가 7인 User만 이 메소드를 사용할 수 있도록 만들면 좋을 것 같습니다.
void Channel::setPwdset(bool set, std::string passwrod){
    this->pwd.setisPasswordSet(set);
    if (this->getPwdSet() == true){
        this->pwd.setPwd(passwrod);
    }
};

//채널 켜기(사람있음, 채널 이름 있음)
void Channel::setIsActive(){
    if (this->ChannelUser.getUserData(0) != this->ChannelUser.end()){
        if(this->ChannelName.length() > 0){
            this->isActive = true;
        }
    }
};

//각 채널에 대해 운영자 만들기
void Channel::setUsersAuth(int ChannelUserID)
{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    if (it != this->ChannelUser.end()){
        it->second->setAuth(7);
    }
};

//유저 지우기
void Channel::eraseUser(int ChannelUserID)
{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    if (it != this->ChannelUser.end()){
        this->ChannelUser.eraseData(ChannelUserID);
    }
};

//채널 켜져있는지, 안켜져있는지
bool Channel::checkAllReady(){
    return (this->isActive);
};

//전체 방송하기
void Channel::broadcast(const std::string& msg, User* from)
{
    for (TotalDatabase<ChannelData>::it it = this->ChannelUser.begin(); it != this->ChannelUser.end(); ++it)
    {
        if (it->second->getWho() == from)
            continue;
        //(*it)->outbox.push(msg); 이거랑 밑에랑 같은 결과를 만듭니다.
        it->second->getWho()->addOutbox(msg);
    }
}

//안에 유저가 있는지 없는지
bool Channel::hasUser(int userId) const {
    return ChannelUser.getUserData(userId) != ChannelUser.end();
}

bool Channel::hasUserGetServerId(int userId) const{
    TotalDatabase<ChannelData>::const_it it  = ChannelUser.begin();
    for (; it != ChannelUser.end(); it++){
        int id = it->second->getWho()->getId();
        if (id == userId){
            return (true);
        }
    }
    return (false);
}

//서버의 아이디를 채널의 아이디로 변환합니다.
int Channel::changeServerIdtoChannel(int userId) const{
    TotalDatabase<ChannelData>::const_it it  = ChannelUser.begin();
    for (; it != ChannelUser.end(); it++){
        int id = it->second->getWho()->getId();
        if (id == userId){
            return (it->second->getid());
        }
    }
    return (-1);
}

//채널 끄기
void Channel::setInactive() {
    isActive = false;
}


//MODE 명령어들
void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}
const std::string& Channel::getTopic() const {
    return _topic;
}
bool Channel::isInviteOnly() const { return inviteOnly; }
bool Channel::isTopicOnly()  const { return topicOnly;  }
int  Channel::getUserLimit() const { return userLimit;  }

void Channel::setInviteOnly(bool v) { inviteOnly = v; }
void Channel::setTopicOnly(bool v)  { topicOnly  = v; }
void Channel::setUserLimit(int v)   { userLimit  = v; }

bool Channel::hasUserById(int uid) const
{
    for (TotalDatabase<ChannelData>::const_it it = ChannelUser.begin();
         it != ChannelUser.end(); ++it)
    {
        SharedPtr<ChannelData> chd = it->second;
        if (chd.is_valid() && chd->getWho()->getId() == uid)
            return true;                      // 중복 발견
    }
    return false;                             // 없음
}

void Channel::ensureOneOp()
{
    // 1) 이미 op 가 남아 있으면 끝
    for (TotalDatabase<ChannelData>::const_it it = ChannelUser.begin();
         it != ChannelUser.end(); ++it)
    {
        SharedPtr<ChannelData> cd = it->second;
        if (cd.is_valid() && cd->getAuth() >= 7)  // op 존재
            return;
    }

    // 2) op 가 없으니 맨 앞 사람(or 임의)에게 부여
    if (ChannelUser.sizeData() == 0) return;      // 채널 비었음

    SharedPtr<ChannelData> first = ChannelUser.begin()->second;
    if (first.is_valid()) {
        first->setAuth(7);

        /* 알림 브로드캐스트 (선택) */
        std::string nick = first->getWho()->getNickName();
        std::string msg  = ":server MODE " + ChannelName +
                           " +o " + nick + "\r\n";
        broadcast(msg, first->getWho());        // 모든 채널 이용자에게 통보
    }
}

bool Channel::checkPassword(std::string pwd){
    return (this->pwd.CheckPassword(pwd));
}