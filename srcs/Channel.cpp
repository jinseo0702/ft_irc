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

void Channel::setId(int id){
    if (id == 0){
        this->ChannelName = "#lobby";
    }
    this->TotalChannelID = id;
};


void Channel::setPwdset(bool set, std::string passwrod){
    this->pwd.setisPasswordSet(set);
    if (this->getPwdSet() == true){
        this->pwd.setPwd(passwrod);
    }
};


void Channel::setIsActive(){
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
        
        it->second->getWho()->addOutbox(msg);
    }
}






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


void Channel::setInactive() {
    isActive = false;
}



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
            return true;                      
    }
    return false;                             
}

void Channel::ensureOneOp()
{
    
    for (TotalDatabase<ChannelData>::const_it it = ChannelUser.begin();
         it != ChannelUser.end(); ++it)
    {
        SharedPtr<ChannelData> cd = it->second;
        if (cd.is_valid() && cd->getAuth() >= 7)  
            return;
    }

    
    if (ChannelUser.sizeData() == 0) return;      

    SharedPtr<ChannelData> first = ChannelUser.begin()->second;
    if (first.is_valid()) {
        first->setAuth(7);

        std::string nick = first->getWho()->getNickName();
        std::string msg  = ":server MODE " + ChannelName +
                           " +o " + nick + "\r\n";
        broadcast(msg, first->getWho());        
    }
}

bool Channel::checkPassword(std::string pwd){
    return (this->pwd.CheckPassword(pwd));
}