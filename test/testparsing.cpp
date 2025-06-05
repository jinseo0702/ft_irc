#include "../include/Parser.hpp"
#include <iostream>

int main(int argc, char *argv[]){

    if (argc == 1)
        return (1);
    Parser par = Parser::parse(argv[1]);
    std::cout << par << std::endl;
    return (0);
}