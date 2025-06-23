#ifndef CHANNEL_HPP
#define CHANNEL_HPP

/*
Channel = 0 은 loby 입니다.
Channel0 은 지워지면 안됩니다.
Channel0의 0번째 User는 모든 채널의 Master 입니다. -> 아마도?
loby는 모든 유저의 정보를 가지고 있습니다.
loby는 password를 가지면 안됩니다.
Loby Channel의 ChannelUser 의 ChannelUserID는 who->id랑 같아야합니다
Channel을 생성하고 관리하는건 User의 명령어를 해석하는 Server가 총괄합니다.
Server는 Chennal의 생성 삭제를 관리 합니다.
Channel의 이름과 Id가 부여되고 0번째ID(관리자가) 생성이되면 Channel은 isActive = true가 되고 사용 가능 합니다.
만약 생성이 된 Channel이 isActive = false인데 생성이 되었다면 Server 는 책임을 지고 Channel을 삭제합니다.
*/

#include <string>
#include <set>
#include "SharedPtr.hpp"
#include "./ChannelData.hpp"
#include "TotalDatabase.hpp"
#include "./Password.hpp"

class Channel
{
    private:
        int TotalChannelID;
        std::string ChannelName;
        TotalDatabase<ChannelData> ChannelUser;
        Password pwd;
        std::string _topic;
        bool isActive;

        // MODE 관련 멤버 추가
        bool inviteOnly;
        std::set<int> _invited;      // ← 초대받은 유저들의 uid
        bool topicOnly;
        int  userLimit;
    public:
        Channel();
        ~Channel();
        
        //make get function
        int getTotalChannelID() const;
        std::string getChannelName() const;
        TotalDatabase<ChannelData>::const_it const getChannelUser(int ChannelUserID) const;
        bool getPwdSet() const;
        bool getIsActive() const;

        //make set Function
        void setId(int id = -1);
        void setName(const std::string &obj);
        void addUser(SharedPtr<User> newUser);
        void setPwdset(bool set = false, std::string passwrod = "noen");
        void setIsActive();
        void setUsersAuth(int ChannelUserID);
        void eraseUser(int ChannelUserID);
        //else
        void broadcast(const std::string& msg, User *from);
        bool checkAllReady();


        //------------------------안현준이 만듬-------------------//
            typedef TotalDatabase<ChannelData>::it UserIt;
    typedef TotalDatabase<ChannelData>::const_it ConstUserIt;

    UserIt userBegin() { return ChannelUser.begin(); }
    UserIt userEnd()   { return ChannelUser.end();   }
    ConstUserIt userBegin() const { return ChannelUser.begin(); }
    ConstUserIt userEnd()   const { return ChannelUser.end();   }
    const TotalDatabase<ChannelData>& getChannelUsers() const { return this->ChannelUser; }
    int getUserCount() const { return this->ChannelUser.sizeData(); }
     void printUserList() const {
        for (TotalDatabase<ChannelData>::const_it it = ChannelUser.begin(); it != ChannelUser.end(); ++it) {
            if (it->second.is_valid() && it->second->getSpUser().is_valid()) {
                User* u = it->second->getSpUser().get();
                std::cout << (u ? u->getNickName() : "?") << "(" << (u ? u->getId() : -1) << ") ";
            }
        }
    }
    bool hasUser(int userId) const;
    void setInactive();
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
       // MODE getters
    bool isInviteOnly() const;
    bool isTopicOnly()  const;
    int  getUserLimit() const;

    // MODE setters
    void setInviteOnly(bool v);
    void setTopicOnly(bool v);
    void setUserLimit(int v);



    //--------------------안현준이 만듬--------/
    bool hasUserById(int uid) const; 
    void ensureOneOp();

    //mode +i
    void addInvite(int uid)      { _invited.insert(uid); }
    void removeInvite(int uid)   { _invited.erase(uid); }
    bool isInvited(int uid) const{ return _invited.count(uid) != 0; }
};

    
#endif
