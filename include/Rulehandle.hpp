#ifndef RULEHANDLE_HPP
#define RULEHANDLE_HPP

#include <map>
#include "./Rule.hpp"
#include <string>
#include <vector>
#include "./Parser.hpp"
#include <utility>

/*
Channel의 명칭은 무조건 #으로 시작합니다.
    message    =  [ ":" prefix SPACE ] command [ params ] crlf
    prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
    command    =  1*letter / 3digit
    params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
               =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]

    nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                    ; any octet except NUL, CR, LF, " " and ":"
    middle     =  nospcrlfcl *( ":" / nospcrlfcl )
    trailing   =  *( ":" / " " / nospcrlfcl )

    SPACE      =  %x20        ; space character
    crlf       =  %x0D %x0A   ; "carriage return" "linefeed"

    
*/

class Parser;

class Rulehandle
{
private:
    static const std::map<std::string, user_role> code;
    static std::map<std::string, user_role> helpCode();
    Rulehandle(){};
    public:
    typedef std::pair<std::string, user_role> Mypair;
    typedef const Mypair const_Mypair;
    Mypair makeErrorPair(const std::string &obj);
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