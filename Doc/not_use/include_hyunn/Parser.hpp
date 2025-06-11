#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>

class Parser
{
  private:
  public:
    Parser(/* args */);
    ~Parser();
    static bool is_command(std::string obj);
};


#endif