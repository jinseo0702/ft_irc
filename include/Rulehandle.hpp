#ifndef RULEHANDLE_HPP
#define RULEHANDLE_HPP

#include <map>
#include "./Rule.hpp"
#include <string>
#include <vector>
#include "./Parser.hpp"
#include <utility>

class Rulehandle
{
private:
    static const std::map<std::string, user_role> code;
    static std::map<std::string, user_role> helpCode();
    Rulehandle(){};
    typedef std::pair<std::string, user_role> Mypair;
    typedef const Mypair const_Mypair;
    Mypair makeErrorPair(const std::string &obj);
    public:
    typedef std::map<std::string, user_role>::iterator it;
    typedef std::map<std::string, user_role>::const_iterator const_it;
    Mypair checkPrefix();
    Mypair checkCommand(const Parser &pars);
    Mypair checkModeOption(const Parser &pars);
    ~Rulehandle() {};
    static Mypair returnPair(const std::string &obj);
    static bool isUserCommand(const user_role role);
    static bool isOperCommand(const user_role role);
    static bool isModeOption(const user_role role);
    static bool isError(const user_role role);
};

#endif