#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <queue>


#define OPERATORS 0x001
#define NORMAL 0x002

class User
{
    private:
        short int user_role;
        bool is_invite;
        bool inchannel;
    public:
        explicit User(int fd = -1);
        int                     fd;
        std::string             ibuf;
        std::queue<std::string> outbox;
};

#endif
