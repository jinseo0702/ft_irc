#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Password.hpp"
#include "../include/SharedPtr.hpp"

//현재 유틸함수들...
Channel* Server::getChannelByName(const std::string& name) {
    for (TotalDatabase<Channel>::it it = _channels.begin(); it != _channels.end(); ++it)
        if (it->second->getChannelName() == name)
            return it->second.get();
    return NULL;
}

User* Server::getUserByNick(const std::string& nick) {
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it)
        if (it->second->getNickName() == nick)
            return it->second.get();
    return NULL;
}

//it return Id 를 반환 fd값이 존재하지 않는다면 -999 값을반환 합니다. 이경우 치명적인 error 입니다.
int    Server::getSamefdUser(const int _pfdsFd){
    for (TotalDatabase<User>::it it = _users.begin(); it != _users.end(); ++it){
        if (it->second->getFd() == _pfdsFd){
            return (it->second->getId());
        }
    }
    return (ERROR_ID);
}

void Server::applyOpFlag(Channel* ch,
                         const std::string& nick,
                         bool give,             // true = +o, false = -o
                         User& src)
{
    User* tgt = getUserByNick(nick);
    if (!tgt || !ch->hasUserById(tgt->getId())) {
        src.numeric(441, nick + " " + ch->getChannelName() +
        " :They aren't on that channel");
        return;
    }
    int changeTgtId = ch->changeServerIdtoChannel(tgt->getId());
    SharedPtr<ChannelData> cd =
        ch->getChannelUsers().getUserData(changeTgtId)->second;
    cd->setAuth(give ? 7 : 0);

    /* MODE echo (op 변경은 곧바로 채널에도 전파) */
    std::string m = ":" + src.fullPrefix() + " MODE " +
                    ch->getChannelName() + (give?" +o ":" -o ") + nick + "\r\n";
    ch->broadcast(m, NULL);
}

void Server::stop() { live = false; }