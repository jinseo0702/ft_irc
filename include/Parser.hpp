#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cctype>
#include "./Rule.hpp"
/*
try catch를 사용하지 않기 위한 노력
Error Code Pattern - 예외 대신 에러 코드나 상태 플래그로 처리
Validity Flag Pattern - 객체 내부에 유효성 상태를 저장
Self-Validating Object Pattern - 객체가 스스로 유효성을 관리
*/

// struct Command
// {
//   std::string target;      // 대상: "#channel" 또는 "nickname" //prefix
//   std::string verb;        // 명령어: "JOIN", "PRIVMSG" 등
//   std::string params;      // 파라미터(메시지 등) Vector로 구현
// };
// static bool is_command(std::string obj);
// static Command parse_line(const std::string& line);
//bool Valid; //코드값을 반환하고록 할까? 생각 중 입니다.



class Parser
{
  private:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    user_role Error;
    bool Valid; //코드값을 반환하고록 할까? 생각 중 입니다.
    int paramsCnt;
    Parser();
  public:
    ~Parser(){};
    static Parser parse(const std::string &line);
    bool isValid() const;
    std::string getPrefix() const;
    std::string getCommand() const;
    const std::vector<std::string> &getParams() const;
    user_role getError() const;
    int parmsCnt() const;
    static void MakeReferToupper(std::string &str);
};

std::ostream& operator<<(std::ostream& out, const Parser& obj);


#endif


//Parse p = Parser(line);
// if(p.isvaild())z
//p ;