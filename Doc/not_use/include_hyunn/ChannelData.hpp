#ifndef CHANNELDATA_HPP
#define CHANNELDATA_HPP

#include "./SharedPtr.hpp"
#include "./user.hpp"

class ChannelData {
    private:
        int ChannelUserID;
        SharedPtr<User> who;
        int auth;
    public:
        ChannelData();
        ChannelData(const ChannelData &obj);
        ChannelData &operator=(const ChannelData &obj);
        ~ChannelData();

        // Getter
        int getid() const;
        int getAuth() const;
        User *getWho() const;

        // Setter
        void setid(int value);
        void setAuth(int value);
        //channel don't have Change autorize
};

#endif