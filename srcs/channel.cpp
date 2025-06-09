#include "../include/channel.hpp"
#include "../include/user.hpp"

Channel::Channel(): TotalChannelID(-1){
    this->isActive = false;
};

Channel::~Channel(){

};

int Channel::getTotalChannelID() const{
    return (this->TotalChannelID);
};

std::string Channel::getChannelName() const{
    return (this->ChannelName);
};

TotalDatabase<ChannelData>::const_it const Channel::getChannelUser(int ChannelUserID) const{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    return (it);
};

bool Channel::getPwdSet() const{
    return (this->pwd.getIsPasswordSet());
};

bool Channel::getIsActive() const{
    return (this->isActive);
};

void Channel::setName(const std::string &obj){
    this->ChannelName = obj;
};

// newUser는 ShardPtr로 Server가 가지고 있는 user명단을 공유합니다. 그러므로 new로 할당할 필요가 없습니다.
void Channel::addUser(SharedPtr<User> newUser){
    // 1. 이미 존재하는지 체크
    for (TotalDatabase<ChannelData>::it it = this->ChannelUser.begin();
         it != this->ChannelUser.end(); ++it) {
        if (it->second->getWho() == newUser.get()) {
            // 이미 이 유저가 들어가 있다면 그냥 리턴!
            return;
        }
    }
    // 2. 신규라면 추가
    this->ChannelUser.addUserWithId(new ChannelData(newUser));

        std::cout << "[ADD USER] " 
              << (newUser.get() ? newUser.get()->getNickName() : "NULL") 
              << " to channel " << this->ChannelName
              << ", user count: " << this->ChannelUser.sizeData()
              << std::endl;

    // 3. 첫 번째 유저(관리자) 권한 부여
    if (this->ChannelUser.getUserData(0) != this->ChannelUser.end()) {
        this->ChannelUser.getUserData(0)->second->setAuth(7);
    }
}
/* void Channel::addUser(SharedPtr<User> newUser){
    this->ChannelUser.addUserWithId(new ChannelData(newUser));
    if (this->ChannelUser.getUserData(0) != this->ChannelUser.end()){
        this->ChannelUser.getUserData(0)->second->setAuth(7);
    }
};
*/
void Channel::setId(int id){
    if (id == 0){
        this->ChannelName = "loby";
    }
    this->TotalChannelID = id;
};

//UserData와 호환이 되도록 만들어야 겠습니다. UserData에서 관리자가 7인 User만 이 메소드를 사용할 수 있도록 만들면 좋을 것 같습니다.
void Channel::setPwdset(bool set, int passwrod){
    this->pwd.setisPasswordSet(set);
    if (this->getPwdSet() == true){
        this->pwd.setPwd(passwrod);
    }
};

void Channel::setIsActve(){
    if (this->ChannelUser.getUserData(0) != this->ChannelUser.end()){
        if(this->ChannelName.length() > 0){
            this->isActive = true;
        }
    }
};

void Channel::setUsersAuth(int ChannelUserID)
{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    if (it != this->ChannelUser.end()){
        it->second->setAuth(7);
    }
};

void Channel::eraseUser(int ChannelUserID)
{
    TotalDatabase<ChannelData>::const_it it = this->ChannelUser.getUserData(ChannelUserID);
    if (it != this->ChannelUser.end()){
        this->ChannelUser.eraseData(ChannelUserID);
    }
};


bool Channel::checkAllReady(){
    return (this->isActive);
};


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
