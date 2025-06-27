#include "../include/Parser.hpp"

Parser::Parser(){
    this->Error = std::make_pair("OK", user_role::OK);
    this->Valid = false;
    this->paramsCnt = 0;
};

// message    =  [ ":" prefix SPACE ] command [ params ] crlf
//prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
bool Parser::CheckPrefix(){
    if (prefix.length() > 0){
        if (Utils::is_servername(prefix)){
            return (true);
        }
        else if(prefix.find('!')){
            std::stringstream is(prefix);
            std::string nick;
            std::getline(is, nick, '!');
            std::string user;
            std::getline(is, user, '@');
            std::string host;
            std::getline(is, host);
            if ((Utils::is_nickname(nick) && Utils::is_user(user) && Utils::is_host(host))){
                return (true);
            }
        }
        else if(prefix.find('@')){
            std::stringstream is(prefix);
            std::string nick;
            std::getline(is, nick, '!');
            std::string host;
            std::getline(is, host);
            if ((Utils::is_nickname(nick) && Utils::is_host(host))){
                return (true);
            }
        }
        else if(Utils::is_nickname(prefix)){
            return (true);
        }
        return (false);
    }
   return (true);
}

// command    =  1*letter
bool Parser::CheckCommand(){
    for (int i = 0; i < this->command.length(); i++){
        unsigned char uc = static_cast<unsigned char>(this->command[i]);
        if (Utils::is_letter(uc) == true){
            continue;
        }
        return(false);
    }
    return (true);
}

// params     =  *15( SPACE middle ) [ SPACE ":" trailing ]
// =/ 15( SPACE middle ) [ SPACE [ ":" ] trailing ]
bool Parser::CheckParams(){
    if (this->paramsCnt > 15){
        return (false);
    }
    if (params.size() > 0){
        std::vector<std::string>::iterator it;
        for (it = params.begin(); it < params.end(); ++it){
            if (Utils::is_middle(*it)){
                continue;
            }
            if(it == (params.end() - 1)){
                if (Utils::trailing(*it) || Utils::is_middle(*it)){
                    continue;
                }
            }
            return(false);
        }
        return (true);
    }
    return (true);
}

bool Parser::finalCheckGrammar(){
    return (CheckPrefix() && CheckCommand() && CheckParams());
}


Parser Parser::parse(const std::string &line){
    Parser par;
    std::string set = Utils::find_first_and_earse(line, " ");
    bool loop = true;
    std::istringstream ss(set);
    
    if (line.empty()){
        par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
        return (par);
    }
    if (line.length() > 512){
        par.Error = Rulehandle::returnPair("ERR_TOOMANYCAHR");
        return (par);
    }
    if (set[0] == ':'){
        if(!(ss >> par.prefix)){
            par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
            return (par);
        }
        par.prefix.erase(0, 1);
    }
    if(!(ss >> par.command)){
        par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
        return (par);
    }
    MakeReferToupper(par.command);
    while (loop){
        std::string temp;
        if(!(ss >> temp)){
            break;
        }
        if (temp[0] == ':'){
            temp.clear();
            temp = line.substr(line.find(':', 1));
            loop = false;
        }
        par.params.push_back(temp);
        par.paramsCnt += 1;
    }
    if (par.paramsCnt > 15){
        par.Error = Rulehandle::returnPair("ERR_TOOMANYTARGETS");
    }
    else{
        par.Valid = true;
    }
    //Final Parsing Grammer Check!
    if (!par.finalCheckGrammar()){
        par.Valid = false;
        par.Error = Rulehandle::returnPair("ERR_FATAL");
    };
    return (par);
};

bool Parser::isValid() const{
    return (this->Valid);
}

std::string Parser::getPrefix() const{
    return (this->prefix);
};

std::string Parser::getCommand() const{
    return (this->command);
};

const std::vector<std::string> &Parser::getParams() const{
    return (this->params);
};

void Parser::MakeReferToupper(std::string &str){
    for (int i = 0; str[i] != '\0'; ++i){
        str[i] = std::toupper(str[i]);
    }
};

std::pair<std::string, user_role> Parser::getError() const{
    return(this->Error);
};

int Parser::parmsCnt() const{
    return(this->paramsCnt);
};


std::ostream& operator<<(std::ostream& out, const Parser& obj)
{
    out << "prefix is = ";
    out << obj.getPrefix();
    out << "\n";
    out << "command is = ";
    out << obj.getCommand();
    out << "\n";
    for (std::vector<std::string>::const_iterator it = obj.getParams().begin(); it != obj.getParams().end(); ++it){
        out << "parmas is = ";
        out << *it;
        out << "\n";
    }
    out << "Error result is = ";
    out << obj.getError().first;
    out << "\n";
    out << "Error Code is = ";
    out << obj.getError().second;
    out << "\n";
    return (out);
};