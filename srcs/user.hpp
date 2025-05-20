#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <queue>

class User
{
    public:
        explicit User(int fd = -1);
        int                     fd;
        std::string             ibuf;
        std::queue<std::string> outbox;
};

#endif
