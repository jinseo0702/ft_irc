#include "channel.hpp"
#include "user.hpp"

void Channel::broadcast(const std::string& msg, User* from)
{
    for (std::set<User*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        if (*it == from)
            continue;
        (*it)->outbox.push(msg);
    }
}
