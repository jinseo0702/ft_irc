#include "../include/Server.hpp"
#include "../include/Signal.hpp"
#include <cstdlib>
#include <iostream>
#include <csignal>

int main(int ac, char** av)
{
    if (ac != 3) 
    {
        std::cerr << "usage: ./ircserv <port> <password>" << std::endl; 
        return 1;
    }
    int port = std::atoi(av[1]);
    std::string password = av[2];
    Server s(port, password);  
    Sig::install(&s);
    s.run();
    return 0;
}
