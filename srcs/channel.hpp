#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>

class User;

class Channel
{
    public:
        std::set<User*> users;
        void broadcast(const std::string& msg, User* from);
};

#endif
