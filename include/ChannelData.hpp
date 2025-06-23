#ifndef CHANNELDATA_HPP
#define CHANNELDATA_HPP

#include "./SharedPtr.hpp"
#include "./User.hpp"
//각 채널 안에 유저 들올떄마다 생김
class ChannelData {
    private:
        int ChannelUserID;
        SharedPtr<User> who;
        int auth;
    public:
        ChannelData(SharedPtr<User> newUser);
        ChannelData(const ChannelData &obj);
        ChannelData &operator=(const ChannelData &obj);
        ~ChannelData();

        // Getter
        int getid() const;
        int getAuth() const;
        User *getWho() const;
        SharedPtr<User> const &getSpUser() const;

        // Setter
        void setId(int value);
        void setAuth(int value);
        void changeUser(SharedPtr<User> newUser);
        //channel don't have Change autorize
};

#endif