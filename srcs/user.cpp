#include "../include/user.hpp"


User::User(int fd, int id) : fd(fd), id(id), newby(0), active(false){
    this->userName = "";
    this->nickName = "";
    this->ibuf = "";
}

User::User(const User &obj) : 
    fd(obj.fd),
    id(obj.id),
    newby(0),
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
        this->newby = obj.newby,
        this->active = obj.active;
        this->userName = obj.userName;
        this->nickName = obj.nickName;
        this->ibuf = obj.ibuf;
        this->outbox = obj.outbox;
    }
    return (*this);
};

User::~User()
{

};


int User::getFd() const
{
    return (this->fd);
};

int User::getId() const
{
    return (this->id);
};

int User::getNewby() const
{
    return (this->newby);
};


bool User::getActive() const
{
    return (this->active);
};

std::string User::getUserName() const
{
    return (this->userName);
};

std::string User::getNickName() const
{
    return (this->nickName);
};

std::string &User::getReferIbuf()
{
    return (this->ibuf);
};

std::queue<std::string> &User::getReferOutbox()
{
    return (this->outbox);
};

std::string User::getIbuf() const
{
    return (this->ibuf);
};

std::queue<std::string> User::getOutbox() const
{
    return (this->outbox);
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
    if ((this->newby ^ 103) && (this->active == false)){
        return (true);
    }
    else{
        return (false);
    }
}

std::ostream& operator<<(std::ostream& out, const User& obj)
{
    out << obj.getFd();
    out << " ";
    out << obj.getId();
    out << " ";
    out << obj.getUserName();
    out << " ";
    out << obj.getNickName();
    out << " ";
    return (out);
}