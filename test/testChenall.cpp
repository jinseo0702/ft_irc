#include "../include/channel.hpp"
#include <string>

/*
test 를 위한 컴파일 방법
clang++ -g -o test ../srcs/user.cpp ../srcs/channel.cpp ../srcs/ChannelData.cpp ../srcs/Password.cpp testChenall.cpp
*/

using namespace std;

int main(void){

    Channel one;

    one.setId(0);
    for (size_t i = 0; i < 4; i++){
        SharedPtr<User> newby(new User(i, i + 1));
        one.addUser(newby);
        newby->setUserName("User name is :" + std::to_string(i));
        newby->setNickName("User Nickname is :" + std::to_string(i));
    }

    Channel two;
    two.setId(1);
    for (size_t i = 0; i < 4; i++){
        two.addUser(one.getChannelUser(i)->second->getSpUser());
    }

    Channel Three;
    Three.setId(1);
    for (size_t i = 0; i < 4; i++){
        Three.addUser(one.getChannelUser(i)->second->getSpUser());
    }

    std::cout << "Chennal name is :" << one.getChannelName() << "\n channel Id is : " << one.getTotalChannelID() << std::endl;
    TotalDatabase<ChannelData>::const_it it = one.getChannelUser(0);
    for (size_t i = 0; i < 4; i++)
    {
        std::cout << *it->second->getWho() <<std::endl;
        std::cout << "Use Count is" << it->second->getSpUser().use_count() <<std::endl;
        std::cout << "Use get is" << it->second->getSpUser().get() <<std::endl;
        std::cout << "Use vaild is" << it->second->getSpUser().is_valid() <<std::endl;
        it++;
    }

    two.~Channel();
    Three.~Channel();

    std::cout << "------------------------------------------------------------------------" << std::endl;
    TotalDatabase<ChannelData>::const_it it2 = one.getChannelUser(0);
    for (size_t i = 0; i < 4; i++)
    {
        std::cout << *it2->second->getWho() <<std::endl;
        std::cout << "Use Count is" << it2->second->getSpUser().use_count() <<std::endl;
        std::cout << "Use get is" << it2->second->getSpUser().get() <<std::endl;
        std::cout << "Use vaild is" << it2->second->getSpUser().is_valid() <<std::endl;
        it2++;
    }

    std::cout << "------------------------------------------------------------------------" << std::endl;
    TotalDatabase<ChannelData>::const_it it3 = one.getChannelUser(0);
    for (size_t i = 0; i < 4; i++)
    {
        std::cout << *it3->second->getWho() <<std::endl;
        int cnt = it3->second->getSpUser().use_count();
        if (cnt > 1){
        std::cout << "Use Count is" << it3->second->getSpUser().use_count() <<std::endl;
        std::cout << "Use get is" << it3->second->getSpUser().get() <<std::endl;
        std::cout << "Use vaild is" << it3->second->getSpUser().is_valid() <<std::endl;
        }
        else{
            std::cout << "All memory is clear and not leak have good day" <<std::endl;
        }
        it3++;
    }
    for (size_t i = 0; i < 4; i++)
    {
        one.eraseUser(i);
    }
    std::cout << "All memory is clear and not leak have good day" <<std::endl;


    
    return (0);
}