#include "../include/Rulehandle.hpp"

const std::map<std::string, user_role> Rulehandle::code = Rulehandle::helpCode();

std::map<std::string, user_role> Rulehandle::helpCode(){
    std::map<std::string, user_role> temp;
    
    
    temp["RPL_WELCOME"] = RPL_WELCOME;
    temp["RPL_YOURHOST"] = RPL_YOURHOST;
    temp["RPL_CREATED"] = RPL_CREATED;
    temp["RPL_MYINFO"] = RPL_MYINFO;

    
    temp["ERR_UNKNOWNERROR"] = ERR_UNKNOWNERROR;
    temp["ERR_NOSUCHNICK"] = ERR_NOSUCHNICK;
    temp["ERR_NOSUCHSERVER"] = ERR_NOSUCHSERVER;
    temp["ERR_NOSUCHCHANNEL"] = ERR_NOSUCHCHANNEL;
    temp["ERR_NOSUCHCOMMAND"] = ERR_NOSUCHCOMMAND;
    temp["ERR_EMPTY"] = ERR_EMPTY;
    temp["ERR_NOSUCHMODEOPTION"] = ERR_NOSUCHMODEOPTION;
    temp["ERR_CANNOTSENDTOCHAN"] = ERR_CANNOTSENDTOCHAN;
    temp["ERR_TOOMANYCHANNELS"] = ERR_TOOMANYCHANNELS;
    temp["ERR_TOOMANYTARGETS"] = ERR_TOOMANYTARGETS;
    temp["ERR_TOOMANYCAHR"] = ERR_TOOMANYCAHR;
    temp["ERR_NICKNAMEINUSE"] = ERR_NICKNAMEINUSE;
    temp["ERR_FATAL"] = ERR_FATAL;
    
    
    temp["OK"] = OK;

    
    temp["JOIN"] = JOIN;
    temp["NICK"] = NICK;
    temp["USER"] = USER;
    temp["QUIT"] = QUIT;
    temp["PART"] = PART;
    temp["PRIVMSG"] = PRIVMSG;
    temp["NOTICE"] = NOTICE;
    temp["PASS"] = PASS;
    temp["LIST"] = LIST;
    temp["SHOW"] = SHOW;

    
    temp["KICK"] = KICK;
    temp["INVITE"] = INVITE;
    temp["TOPIC"] = TOPIC;
    temp["MODE"] = MODE;

    
    temp["+i"] = MODE_INVITESET;
    temp["-i"] = MODE_INVITERM;
    temp["+t"] = MODE_TOPICSET;
    temp["-t"] = MODE_TOPICRM;
    temp["+k"] = MODE_KEYSET;
    temp["-k"] = MODE_KEYRM;
    temp["+o"] = MODE_OWNERGIVE;
    temp["-o"] = MODE_OWNERTAKE;
    temp["+l"] = MODE_LIMITSET;
    temp["-l"] = MODE_LIMITRM;

    
    temp["DCC"] = DCC_SEND;
    temp["DCC_SEND"] = DCC_SEND;
    temp["DCC_ACCEPT"] = DCC_ACCEPT;
    temp["DCC_RESUME"] = DCC_RESUME;
    temp["DCC_REJECT"] = DCC_REJECT;

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



