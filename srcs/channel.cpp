#include "../include/channel.hpp"
#include "../include/user.hpp"

void Channel::broadcast(const std::string& msg, User* from)
{
    for (std::set<User *>::iterator it = this->users.begin(); it != this->users.end(); ++it)
    {
        if (*it == from)
            continue;
        (*it)->outbox.push(msg);
    }
}
