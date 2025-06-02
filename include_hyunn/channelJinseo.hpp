#ifndef CHANNELJINSEO_HPP
#define CHANNELJINSEO_HPP

/*
Channel = 0 은 loby 입니다.
Channel0 은 지워지면 안됩니다.
Channel0의 0번째 User는 모든 채널의 Master 입니다. -> 아마도?
loby는 모든 유저의 정보를 가지고 있습니다.
loby는 password를 가지면 안됩니다.
*/

#include <set>
#include <string>
#include "SharedPtr.hpp"
#include "./ChannelData.hpp"
#include "TotalDatabase.hpp"
#include "./Password.hpp"

class User;

class ChannelJinseo
{
    private:

        int TotalChannelID;
        std::string ChannelName;
        TotalDatabase<ChannelData> ChannelUser;
        Password pwd;
    public:
        void broadcast(const std::string& msg, User *from);
};

#endif
