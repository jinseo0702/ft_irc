#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>

struct Command
{
  std::string target;      // 대상: "#channel" 또는 "nickname" //prefix
  std::string verb;        // 명령어: "JOIN", "PRIVMSG" 등
  std::string params;      // 파라미터(메시지 등) Vector로 구현
};

class Parser
{
  private:
  public:
    Parser(/* args */);
    ~Parser();
    static bool is_command(std::string obj);
    static Command parse_line(const std::string& line);
};


#endif