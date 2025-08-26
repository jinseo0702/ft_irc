#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <queue>
#include <iostream>
#include <sstream>
#include <iomanip>

class User
{
    private:
        int fd;
        int id;
        int newby;
        bool active;
        std::string userName;
        std::string nickName;
        std::string ibuf;
        std::queue<std::string>	outbox;
    public:
        explicit User(int fd = -1, int id = -1);
                User(const User &obj);
                User &operator=(const User &obj);
                ~User();
                
                int getFd() const;
                int getId() const;
                int getNewby() const;
                bool getActive() const;
                std::string getUserName() const;
                std::string getNickName() const;
				std::string &getReferIbuf();
                std::queue<std::string> &getReferOutbox();
                std::string getIbuf() const;
                std::queue<std::string> getOutbox() const;

                void setFd(int sfd);
                void setId(int sid);
                void setNewby(int orcal);
                void setActive(bool sactive);
                void setUserName(const std::string &sUserName);
                void setNickName(const std::string &sNickName);
                void setIbuf(const std::string &sIbuf);
                void addOutbox(const std::string& message);

                void numeric(int code, const std::string& params);
                std::string fullPrefix() const; 
                bool is_newby();
};

std::ostream& operator<<(std::ostream& out, const User& obj);

#endif
