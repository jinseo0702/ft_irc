#include "../include/Parser.hpp"

// "JOIN #dev" → true 명령어 맞음
// "Hello"     → false 명령어 아님

//근데 여기서 문제 만약에 JOIN같은 명령어 뒤에 잘못된게 온다면? 혹은 없는 경로가 온다면?


/*
bool Parser::is_command(const std::string& obj)
{
    if (obj.empty())
        return false;
    // IRC 명령어는 대개 대문자 + 공백으로 시작함
    std::istringstream iss(obj);
    std::string word;
    iss >> word;
    if (word.empty()) return false;
    // 대표 명령어 체크
    static const std::string cmds[] = {
        "JOIN", "PRIVMSG", "PART", "NICK", "USER", "QUIT", "MODE", "KICK", "INVITE", "TOPIC"
    };
    for (size_t i = 0; i < sizeof(cmds)/sizeof(cmds[0]); ++i) {
        if (word == cmds[i])
            return true;
    }
    return false;
}



Command Parser::parse_line(const std::string& line) {
    Command cmd;
    // 1. 명령어(verb) 추출
    std::istringstream iss(line);
    iss >> cmd.verb;

    // 2. 대상(target) 추출 (있으면)
    if (cmd.verb == "JOIN" || cmd.verb == "PRIVMSG" || cmd.verb == "PART")
        iss >> cmd.target;

    // 3. 나머지(메시지 등) 추출
    std::getline(iss, cmd.params);

    // 앞 공백/콜론 처리
    if (!cmd.params.empty() && cmd.params[0] == ' ')
        cmd.params.erase(0, 1);
    if (!cmd.params.empty() && cmd.params[0] == ':')
        cmd.params.erase(0, 1);

    // 4. 구조체 통째로 리턴!

    //디버깅용으로 하나 만듬
    std::cout << "verb: " << cmd.verb << std::endl;
    std::cout << "target: " << cmd.target << std::endl;
    std::cout << "params: " << cmd.params << std::endl;


    return cmd;
}

*/
Parser::Parser(){
    this->Error = user_role::OK;
    this->Valid = false;
    this->paramsCnt = 0;
};

Parser Parser::parse(const std::string &line){
    Parser par;
    std::istringstream ss(line);
    bool loop = true;
    
    if (line.empty()){
        par.Error = ERR_UNKNOWNERROR;
        return (par);
    }
    if (line.length() > 512){
        par.Error = ERR_TOOMANYCAHR;
        return (par);
    }
    if (line[0] == ':'){
        if(!(ss >> par.prefix)){
            par.Error = ERR_UNKNOWNERROR;
            return (par);
        }
        par.prefix.erase(0, 1);
    }
    if(!(ss >> par.command)){
        par.Error = ERR_UNKNOWNERROR;
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
    if (par.paramsCnt < 15){
        par.Error = ERR_TOOMANYTARGETS;
        par.Valid = true;
    }
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

user_role Parser::getError() const{
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
    
    return (out);
};