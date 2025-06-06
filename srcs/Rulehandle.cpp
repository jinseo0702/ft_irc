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

    //user command 100 ~ 106
    temp["JOIN"] = user_role::JOIN;
    temp["NICK"] = user_role::NICK;
    temp["USER"] = user_role::USER;
    temp["QUIT"] = user_role::QUIT;
    temp["PART"] = user_role::PART;
    temp["PRIVMSG"] = user_role::PRIVMSG;
    temp["NOTICE"] = user_role::NOTICE;

    //oper command 1000 ~ 1004
    temp["KICK"] = user_role::KICK;
    temp["INVITE"] = user_role::INVITE;
    temp["TOPIC"] = user_role::TOPIC;
    temp["MODE"] = user_role::MODE;

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

Rulehandle::Mypair Rulehandle::makeErrorPair(const std::string &obj){
    Rulehandle::const_it it;

    it = this->code.find(obj);
    std::make_pair(it->first, it->second);
};


Rulehandle::Mypair Rulehandle::checkCommand(const Parser &pars){
    Rulehandle::const_it it;
    it = this->code.find(pars.getCommand());
    if (it == this->code.end()){
        return (makeErrorPair("ERR_NOSUCHCOMMAND"));
    }
    if ((it->second <= 106 && it->second >= 100) || (it->second <= 1000 && it->second >= 1004)){
        return (std::make_pair(it->first, it->second));
    }
    return (makeErrorPair("ERR_NOSUCHCOMMAND"));
};

Rulehandle::Mypair Rulehandle::checkModeOption(const Parser &pars){
    std::vector<std::string> temp = pars.getParams();
    Rulehandle::const_it it;
    if (temp.empty()){
        it = this->code.find("ERR_EMPTY");
        return (std::make_pair(it->first, it->second));
    }
    it = this->code.find(temp[1]);
    temp[1];
};





