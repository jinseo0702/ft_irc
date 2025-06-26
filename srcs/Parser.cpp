#include "../include/Parser.hpp"

Parser::Parser(){
    this->Error = std::make_pair("OK", OK);
    this->Valid = false;
    this->paramsCnt = 0;
};

// message    =  [ ":" prefix SPACE ] command [ params ] crlf
//prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
bool Parser::CheckPrefix(){
    if (prefix.empty()){
        return true;
    }
    
    if (Utils::is_servername(prefix)){
        return true;
    }
    
    size_t at_pos = prefix.find('@');
    size_t bang_pos = prefix.find('!');
    
    if (bang_pos != std::string::npos){
        std::string nick = prefix.substr(0, bang_pos);
        if (at_pos != std::string::npos && at_pos > bang_pos){
            std::string user = prefix.substr(bang_pos + 1, at_pos - bang_pos - 1);
            std::string host = prefix.substr(at_pos + 1);
            return (Utils::is_nickname(nick) && Utils::is_user(user) && Utils::is_host(host));
        }
    }
    else if (at_pos != std::string::npos){
        std::string nick = prefix.substr(0, at_pos);
        std::string host = prefix.substr(at_pos + 1);
        return (Utils::is_nickname(nick) && Utils::is_host(host));
    }
    
    return Utils::is_nickname(prefix);
}

// command    =  1*letter
bool Parser::CheckCommand(){
    for (size_t i = 0; i < this->command.length(); ++i){
        if (!Utils::is_letter(static_cast<unsigned char>(this->command[i]))){
            return false;
        }
    }
    return true;
}

// params     =  *15( SPACE middle ) [ SPACE ":" trailing ]
// =/ 15( SPACE middle ) [ SPACE [ ":" ] trailing ]
bool Parser::CheckParams(){
    if (this->paramsCnt > 15){
        return false;
    }
    
    if (params.empty()){
        return true;
    }
    
    for (size_t i = 0; i < params.size(); ++i){
        if (i == params.size() - 1){
            if (!Utils::trailing(params[i]) && !Utils::is_middle(params[i])){
                return false;
            }
        } else {
            if (!Utils::is_middle(params[i])){
                return false;
            }
        }
    }
    return true;
}

bool Parser::finalCheckGrammer(){
    return (CheckPrefix() && CheckCommand() && CheckParams());
}

Parser Parser::parse(const std::string &line){
    Parser par;
    
    if (line.empty()){
        par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
        return par;
    }
    
    if (line.length() > 512){
        par.Error = Rulehandle::returnPair("ERR_TOOMANYCAHR");
        return par;
    }
    
    std::string set = Utils::find_first_and_earse(line, " ");
    std::istringstream ss(set);
    
    // Parse prefix
    if (set[0] == ':'){
        if(!(ss >> par.prefix)){
            par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
            return par;
        }
        par.prefix.erase(0, 1);
    }
    
    // Parse command
    if(!(ss >> par.command)){
        par.Error = Rulehandle::returnPair("ERR_UNKNOWNERROR");
        return par;
    }
    MakeReferToupper(par.command);
    
    // Parse parameters
    std::string temp;
    while (ss >> temp){
        if (temp[0] == ':'){
            temp = line.substr(line.find(':', 1));
            par.params.push_back(temp);
            par.paramsCnt++;
            break;
        }
        par.params.push_back(temp);
        par.paramsCnt++;
    }
    
    if (par.paramsCnt > 15){
        par.Error = Rulehandle::returnPair("ERR_TOOMANYTARGETS");
    } else {
        par.Valid = true;
    }
    
    // Final parsing grammar check
    if (!par.finalCheckGrammer()){
        par.Valid = false;
        par.Error = Rulehandle::returnPair("ERR_FATAL");
    }
    
    return par;
};

bool Parser::isValid() const{
    return this->Valid;
}

std::string Parser::getPrefix() const{
    return this->prefix;
};

std::string Parser::getCommand() const{
    return this->command;
};

const std::vector<std::string> &Parser::getParams() const{
    return this->params;
};

void Parser::MakeReferToupper(std::string &str){
    for (size_t i = 0; i < str.length(); ++i){
        str[i] = std::toupper(str[i]);
    }
};

std::pair<std::string, user_role> Parser::getError() const{
    return this->Error;
};

int Parser::parmsCnt() const{
    return this->paramsCnt;
};

std::ostream& operator<<(std::ostream& out, const Parser& obj)
{
    out << "prefix is = " << obj.getPrefix() << "\n";
    out << "command is = " << obj.getCommand() << "\n";
    
    for (std::vector<std::string>::const_iterator it = obj.getParams().begin(); 
         it != obj.getParams().end(); ++it){
        out << "params is = " << *it << "\n";
    }
    
    out << "Error result is = " << obj.getError().first << "\n";
    out << "Error Code is = " << obj.getError().second << "\n";
    return out;
};