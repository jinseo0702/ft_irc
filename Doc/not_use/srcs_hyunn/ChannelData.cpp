#include "../include/ChannelData.hpp"

ChannelData::ChannelData(SharedPtr<User> newUser): ChannelUserID(-1){
    this->who = newUser;
    this->auth = -1;
};

ChannelData::ChannelData(const ChannelData &obj):
ChannelUserID(obj.ChannelUserID),
who(obj.who),
auth(obj.auth){

};

ChannelData &ChannelData::operator=(const ChannelData &obj){
    if(this != &obj){
        this->ChannelUserID = obj.ChannelUserID;
        this->who = obj.who;
        this->auth = obj.auth;
    }
    return (*this);
};

ChannelData::~ChannelData(){

};

int ChannelData::getid() const{
    return (this->ChannelUserID);
};

int ChannelData::getAuth() const{
    return (this->auth);
};

User *ChannelData::getWho() const{
    return (this->who.get());
};

SharedPtr<User> const &ChannelData::getSpUser() const{
    return (this->who);
};

void ChannelData::setId(int value){
    this->ChannelUserID = value;
};

void ChannelData::setAuth(int value){
    //이 부분은 고려를 해야하는게 있습니다. 나중에 명령어를 실행 할때, 권한이 있는 사람만 이 setAuth 함수를 사용가능합니다.
    this->auth = value;
};