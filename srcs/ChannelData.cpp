#include "../include/ChannelData.hpp"

//권한 7은 관리자를 뜻합니다. 생성자
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

//각 채널 유저 아이디 가져오기(채널 데이터 안에 있음)
int ChannelData::getid() const{
    return (this->ChannelUserID);
};

//각 채널 유저 권한정보 가져오기(채널 데이터 안에 있음)
int ChannelData::getAuth() const{
    return (this->auth);
};

//각 채널 유저의 정보 가져오기 인데 진짜 1명 가져오는거임 보안문제 있음 포인터(수정가능)
User *ChannelData::getWho() const{
    return (this->who.get());
};

//각 채널 유저의 정보 가져오기 인데 진짜 1명 가져오는거임 보안문제 있음 참조자(못고침)
SharedPtr<User> const &ChannelData::getSpUser() const{
    return (this->who);
};

//각 채널 유저 아이디 세팅(채널 데이터 안에 있음)
void ChannelData::setId(int value){
    this->ChannelUserID = value;
};

//각 채널 유저 권한정보 설정(채널 데이터 안에 있음)
void ChannelData::setAuth(int value){
    //이 부분은 고려를 해야하는게 있습니다. 나중에 명령어를 실행 할때, 권한이 있는 사람만 이 setAuth 함수를 사용가능합니다.
    this->auth = value;
};

//유저 바꾸기(최적화인데 안쓰는 공간 쓰게 하려고)
void ChannelData::changeUser(SharedPtr<User> newUser){
    this->who = newUser;
};