#include "../include/Channel.hpp"
#include "../include/User.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/SharedPtr.hpp"
#include "../include/Password.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"

// 상수 정의
namespace {
    const int SUPER_USER_FD = 777;
    const int SUPER_USER_NEWBY = 103;
    const std::string SUPER_USER_NAME = "Super";
    const std::string LOCALHOST = "localhost";
}

User::User(int fd, int id) : fd(fd), id(id), newby(0), active(false){
    if (fd == SUPER_USER_FD){
        this->userName = SUPER_USER_NAME;
        this->nickName = SUPER_USER_NAME;
        this->fd = 0;
        this->newby = SUPER_USER_NEWBY;
        this->active = true;
    }
    else{
        this->userName = "";
        this->nickName = "";
    }
    this->ibuf = "";
}

User::User(const User &obj) : 
    fd(obj.fd),
    id(obj.id),
    newby(obj.newby),
    active(obj.active),
    userName(obj.userName),
    nickName(obj.nickName),
    ibuf(obj.ibuf),
    outbox(obj.outbox)
{
};

User &User::operator=(const User &obj)
{
    if (this != &obj){        
        this->fd = obj.fd;
        this->id = obj.id;
        this->newby = obj.newby;
        this->active = obj.active;
        this->userName = obj.userName;
        this->nickName = obj.nickName;
        this->ibuf = obj.ibuf;
        this->outbox = obj.outbox;
    }
    return *this;
};

User::~User()
{
};

int User::getFd() const
{
    return this->fd;
};

int User::getId() const
{
    return this->id;
};

int User::getNewby() const
{
    return this->newby;
};

bool User::getActive() const
{
    return this->active;
};

std::string User::getUserName() const
{
    return this->userName;
};

std::string User::getNickName() const
{
    return this->nickName;
};

std::string &User::getReferIbuf()
{
    return this->ibuf;
};

std::queue<std::string> &User::getReferOutbox()
{
    return this->outbox;
};

std::string User::getIbuf() const
{
    return this->ibuf;
};

std::queue<std::string> User::getOutbox() const
{
    return this->outbox;
};

void User::setFd(int sfd)
{
    this->fd = sfd;
};

void User::setId(int sid)
{
    this->id = sid;
};

void User::setNewby(int orcal)
{
    this->newby |= orcal;
};

void User::setActive(bool sactive)
{
    this->active = sactive;
};

void User::setUserName(const std::string &sUserName)
{
    this->userName = sUserName;
};

void User::setNickName(const std::string &sNickName)
{
    this->nickName = sNickName;
};

void User::setIbuf(const std::string &sIbuf)
{
    this->ibuf = sIbuf;
};

void User::addOutbox(const std::string& message)
{
    this->outbox.push(message);
};

bool User::is_newby(){
    return ((this->newby ^ SUPER_USER_NEWBY) && !this->active);
}

std::ostream& operator<<(std::ostream& out, const User& obj)
{
    out << obj.getFd() << " "
        << obj.getId() << " "
        << obj.getUserName() << " "
        << obj.getNickName() << " ";
    return out;
}

void User::numeric(int code, const std::string& params)
{
    std::ostringstream oss;
    oss << ":server "                                 // 서버 프리픽스
        << std::setw(3) << std::setfill('0')          // 001 같은 3자리
        << code << ' '
        << nickName << ' '                          // 대상 닉
        << params << "\r\n";
    addOutbox(oss.str());
}

std::string User::fullPrefix() const
{
    return nickName + "!" + userName + "@" + LOCALHOST + " ";
}