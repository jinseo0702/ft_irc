#include "../include/Parser.hpp"

// "JOIN #dev" → true 명령어 맞음
// "Hello"     → false 명령어 아님

//근데 여기서 문제 만약에 JOIN같은 명령어 뒤에 잘못된게 온다면? 혹은 없는 경로가 온다면?

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