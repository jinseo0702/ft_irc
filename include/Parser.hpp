#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cctype>
#include <utility>
#include "./Rule.hpp"
#include "./Rulehandle.hpp"
#include "./Utils.hpp"

class Parser
{
  private:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::pair<std::string, user_role> Error;
    bool Valid; //코드값을 반환하고록 할까? 생각 중 입니다.
    int paramsCnt;
    Parser();
    bool CheckPrefix();
    bool CheckCommand();
    bool CheckParams();
    bool finalCheckGrammer();
  public:
    ~Parser(){};
    static Parser parse(const std::string &line);
    bool isValid() const;
    std::string getPrefix() const;
    std::string getCommand() const;
    const std::vector<std::string> &getParams() const;
    std::pair<std::string, user_role> getError() const;
    int parmsCnt() const;
    static void MakeReferToupper(std::string &str);
};

std::ostream& operator<<(std::ostream& out, const Parser& obj);


#endif
