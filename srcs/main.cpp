#include "../include/Server.hpp"
#include "../include/signal.hpp"
#include <cstdlib>
#include <iostream>
#include <csignal>

int main(int ac, char** av)
{
    if (ac != 3) // 인자 개수 3개 (프로그램명 + 포트 + 패스워드)
    {
        std::cerr << "usage: ./ircserv <port> <password>" << std::endl; 
        return 1;
    }
    int port = std::atoi(av[1]);
    std::string password = av[2];
    Server s(port, password);  // 패스워드도 넘김
    sig::install(&s);
    s.run();
    return 0;
}
