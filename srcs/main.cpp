#include "../include/server.hpp"
#include <cstdlib>
#include <iostream>

int main(int ac, char** av)
{
    if (ac != 2)
    {
        std::cerr << "usage: ./ircserv <port>" << std::endl; 
        return 1;
    }
    Server s(std::atoi(av[1]));
    s.run();
    return 0;
}
