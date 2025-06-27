#include "../include/Rulehandle.hpp"

const std::map<std::string, user_role> Rulehandle::code = Rulehandle::helpCode();

std::map<std::string, user_role> Rulehandle::helpCode(){
    std::map<std::string, user_role> temp;
    
    //RPL 1 ~ 4;
    temp["RPL_WELCOME"] = user_role::RPL_WELCOME;
    temp["RPL_YOURHOST"] = user_role::RPL_YOURHOST;
    temp["RPL_CREATED"] = user_role::RPL_CREATED;
    temp["RPL_MYINFO"] = user_role::RPL_MYINFO;

    //ERR 400 ~ 412
    temp["ERR_UNKNOWNERROR"] = user_role::ERR_UNKNOWNERROR;
    temp["ERR_NOSUCHNICK"] = user_role::ERR_NOSUCHNICK;
    temp["ERR_NOSUCHSERVER"] = user_role::ERR_NOSUCHSERVER;
    temp["ERR_NOSUCHCHANNEL"] = user_role::ERR_NOSUCHCHANNEL;
    temp["ERR_NOSUCHCOMMAND"] = user_role::ERR_NOSUCHCOMMAND;
    temp["ERR_EMPTY"] = user_role::ERR_EMPTY;
    temp["ERR_NOSUCHMODEOPTION"] = user_role::ERR_NOSUCHMODEOPTION;
    temp["ERR_CANNOTSENDTOCHAN"] = user_role::ERR_CANNOTSENDTOCHAN;
    temp["ERR_TOOMANYCHANNELS"] = user_role::ERR_TOOMANYCHANNELS;
    temp["ERR_TOOMANYTARGETS"] = user_role::ERR_TOOMANYTARGETS;
    temp["ERR_TOOMANYCAHR"] = user_role::ERR_TOOMANYCAHR;
    temp["ERR_NICKNAMEINUSE"] = user_role::ERR_NICKNAMEINUSE;
    temp["ERR_FATAL"] = user_role::ERR_FATAL;
    
    //OK 777
    temp["OK"] = user_role::OK;

    //user command 100 ~ 109
    temp["JOIN"] = user_role::JOIN;
    temp["NICK"] = user_role::NICK;
    temp["USER"] = user_role::USER;
    temp["QUIT"] = user_role::QUIT;
    temp["PART"] = user_role::PART;
    temp["PRIVMSG"] = user_role::PRIVMSG;
    temp["NOTICE"] = user_role::NOTICE;
    temp["PASS"] = user_role::PASS;
    temp["LIST"] = user_role::LIST;
    temp["SHOW"] = user_role::SHOW;

    //oper command 1000 ~ 1003
    temp["KICK"] = user_role::KICK;
    temp["INVITE"] = user_role::INVITE;
    temp["TOPIC"] = user_role::TOPIC;
    temp["MODE"] = user_role::MODE;

    //DCC command 2000 ~ 2003
    temp["DCC"] = user_role::DCC_SEND;
    temp["DCC_SEND"] = user_role::DCC_SEND;
    temp["DCC_ACCEPT"] = user_role::DCC_ACCEPT;
    temp["DCC_RESUME"] = user_role::DCC_RESUME;
    temp["DCC_REJECT"] = user_role::DCC_REJECT;

    //mode command 2001 ~ 2010
    temp["+i"] = user_role::MODE_INVITESET;
    temp["-i"] = user_role::MODE_INVITERM;
    temp["+t"] = user_role::MODE_TOPICSET;
    temp["-t"] = user_role::MODE_TOPICRM;
    temp["+k"] = user_role::MODE_KEYSET;
    temp["-k"] = user_role::MODE_KEYRM;
    temp["+o"] = user_role::MODE_OWNERGIVE;
    temp["-o"] = user_role::MODE_OWNERTAKE;
    temp["+l"] = user_role::MODE_LIMITSET;
    temp["-l"] = user_role::MODE_LIMITRM;

    return (temp);
};

Rulehandle::Mypair Rulehandle::makeErrorPair(const std::string &obj) {
    Rulehandle::const_it it = Rulehandle::code.find(obj);
    return std::make_pair(it->first, it->second);
}

Rulehandle::Mypair Rulehandle::checkCommand(const Parser &pars) {
    Rulehandle::const_it it;
    if (isError(pars.getError().second)) {
        return pars.getError();
    }
    it = Rulehandle::code.find(pars.getCommand());
    if (it == Rulehandle::code.end()) {
        return Rulehandle::makeErrorPair("ERR_NOSUCHCOMMAND");
    }
    if (isUserCommand(it->second) || isOperCommand(it->second)) {
        return Rulehandle::returnPair(it->first);
    }
    return Rulehandle::makeErrorPair("ERR_NOSUCHCOMMAND");
}

Rulehandle::Mypair Rulehandle::checkModeOption(const Parser &pars) {
    std::vector<std::string> temp = pars.getParams();
    Rulehandle::const_it it;

    if (isError(pars.getError().second)) {
        return pars.getError();
    }
    if (temp.empty()) {
        it = Rulehandle::code.find("ERR_EMPTY");
        return Rulehandle::makeErrorPair("ERR_EMPTY");
    }
    it = Rulehandle::code.find(temp[0]);
    if (it == Rulehandle::code.end()) {
        return Rulehandle::makeErrorPair("ERR_NOSUCHCOMMAND");
    }
    else if (isModeOption(it->second) == false) {
        return Rulehandle::makeErrorPair("ERR_NOSUCHCOMMAND");
    }
    return Rulehandle::returnPair(it->first);
}

Rulehandle::Mypair Rulehandle::returnPair(const std::string &obj){
    Rulehandle temp;
    Rulehandle::const_it it;

    it = temp.code.find(obj);
    if (it == temp.code.end()){
        it = temp.code.find("ERR_FATAL");
        return (std::make_pair(it->first, it->second));
    }
    return(std::make_pair(it->first, it->second));
};

bool Rulehandle::isUserCommand(const user_role role){
    if (role >= 100 && role <= 109){
        return(true);
    }
    return (false);
};

bool Rulehandle::isOperCommand(const user_role role){
    if (role >= 1000 && role <= 1003){
        return(true);
    }
    if (role >= 2000 && role <= 2003){ // DCC 명령어들
        return(true);
    }
    return (false);
};

bool Rulehandle::isModeOption(const user_role role){
    if (role >= 2001 && role <= 2010){
        return(true);
    }
    return (false);
};

bool Rulehandle::isError(const user_role role){
    if (role >= 400 && role <= 412){
        return(true);
    }
    return (false);
};



