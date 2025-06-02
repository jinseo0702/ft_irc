#include "../include/TotalDatabase.hpp"
#include "../include/user.hpp"

/*
Tset 를 위한 컴파일 방법
clang++ -g -o test ../srcs/user.cpp ../srcs/channel.cpp ../srcs/ChannelData.cpp ../srcs/Password.cpp testTotalDatabase.cpp
*/

int main(void){

    TotalDatabase<User> UserData;

    for (size_t i = 0; i < 1000; i++)
    {
        UserData.addUserWithId(new User(i, i + 1));
        TotalDatabase<User>::it it = UserData.getUserData(i);
        std::string name = "Local" + std::to_string(i);
        it->second->setUserName(name);
        std::string nickname = "NickNmae" + std::to_string(i);
        it->second->setNickName(nickname);
    }

    TotalDatabase<User>::it it = UserData.getUserData(0);
    for (size_t i = 0; i < 1000; i++)
    {
        std::cout << *it->second <<std::endl;
        it++;
    }
    

    return (0);
}

