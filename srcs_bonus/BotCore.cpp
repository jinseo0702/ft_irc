#include "../include/Bot.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

Bot::Bot() : _active(false), _commandCount(0) {
    _startTime = time(NULL);
    initialize();
}

Bot::~Bot() {
}

// 기본 설정
void Bot::setNickname(const std::string& nick) { _nickname = nick; }
void Bot::setUsername(const std::string& user) { _username = user; }
void Bot::setRealname(const std::string& real) { _realname = real; }
void Bot::setVersion(const std::string& ver) { _version = ver; }
void Bot::setDescription(const std::string& desc) { _description = desc; }
void Bot::setActive(bool active) { _active = active; }

// Getters
std::string Bot::getNickname() const { return _nickname; }
std::string Bot::getUsername() const { return _username; }
std::string Bot::getRealname() const { return _realname; }
std::string Bot::getVersion() const { return _version; }
std::string Bot::getDescription() const { return _description; }
bool Bot::isActive() const { return _active; }
std::queue<std::string>& Bot::getOutbox() { return _outbox; }

void Bot::initialize() {
    setNickname("ft_irc_bot");
    setUsername("bot");
    setRealname("ft_irc Bot v1.0");
    setVersion("1.0");
    setDescription("A simple IRC bot for ft_irc server");
    setActive(true);
    
    setupCommands();
    loadQuotes();
    loadEightBallResponses();
    loadWeatherData();
}

void Bot::setupCommands() {
    // 명령어 등록
    // 1. BotCommand 구조체들을 먼저 선언합니다.
    BotCommand helpCmd;
    BotCommand timeCmd;
    BotCommand weatherCmd;
    BotCommand calcCmd;
    BotCommand quoteCmd;
    BotCommand rollCmd;
    BotCommand eightBallCmd;

    // 2. 각 구조체의 멤버 변수들을 채워줍니다.
    helpCmd.type = BOT_HELP;
    helpCmd.command = "help";
    helpCmd.description = "도움말을 보여줍니다";
    helpCmd.aliases.push_back("h"); // push_back 사용
    helpCmd.aliases.push_back("?"); // push_back 사용

    timeCmd.type = BOT_TIME;
    timeCmd.command = "time";
    timeCmd.description = "현재 시간을 보여줍니다";
    timeCmd.aliases.push_back("clock");

    weatherCmd.type = BOT_WEATHER;
    weatherCmd.command = "weather";
    weatherCmd.description = "날씨 정보를 보여줍니다";
    weatherCmd.aliases.push_back("w");

    calcCmd.type = BOT_CALC;
    calcCmd.command = "calc";
    calcCmd.description = "수식을 계산합니다";
    calcCmd.aliases.push_back("calculate");
    calcCmd.aliases.push_back("c");

    quoteCmd.type = BOT_QUOTE;
    quoteCmd.command = "quote";
    quoteCmd.description = "랜덤 명언을 보여줍니다";
    quoteCmd.aliases.push_back("q");

    rollCmd.type = BOT_ROLL;
    rollCmd.command = "roll";
    rollCmd.description = "주사위를 굴립니다";
    rollCmd.aliases.push_back("dice");

    eightBallCmd.type = BOT_8BALL;
    eightBallCmd.command = "8ball";
    eightBallCmd.description = "8번 공에게 질문합니다";
    eightBallCmd.aliases.push_back("8");
    
    _commands.push_back(helpCmd);
    _commands.push_back(timeCmd);
    _commands.push_back(weatherCmd);
    _commands.push_back(calcCmd);
    _commands.push_back(quoteCmd);
    _commands.push_back(rollCmd);
    _commands.push_back(eightBallCmd);
    
    // 명령어 매핑 생성
    for (std::vector<BotCommand>::iterator it = _commands.begin(); 
         it != _commands.end(); ++it) {
        _commandMap[it->command] = it->type;
        for (std::vector<std::string>::iterator alias = it->aliases.begin();
             alias != it->aliases.end(); ++alias) {
            _commandMap[*alias] = it->type;
        }
    }
}

void Bot::loadQuotes() {
    _initializeDefaultQuotes();
}

void Bot::loadEightBallResponses() {
    _initializeEightBallResponses();
}

void Bot::loadWeatherData() {
    _initializeWeatherData();
}

void Bot::_initializeDefaultQuotes() {
    _quotes.push_back("프로그래밍은 예술이다.");
    _quotes.push_back("코드는 읽기 쉬워야 한다.");
    _quotes.push_back("버그는 항상 있다.");
    _quotes.push_back("최적화는 나중에 하라.");
    _quotes.push_back("간단함이 최고다.");
    _quotes.push_back("테스트는 중요하다.");
    _quotes.push_back("문서화를 게을리하지 마라.");
    _quotes.push_back("리팩토링은 필수다.");
    _quotes.push_back("함수는 한 가지 일만 해야 한다.");
    _quotes.push_back("네이밍이 중요하다.");
}

void Bot::_initializeEightBallResponses() {
    _eightBallResponses.push_back("그렇습니다.");
    _eightBallResponses.push_back("아니오.");
    _eightBallResponses.push_back("아마도.");
    _eightBallResponses.push_back("확실합니다.");
    _eightBallResponses.push_back("절대 아닙니다.");
    _eightBallResponses.push_back("나중에 다시 물어보세요.");
    _eightBallResponses.push_back("그럴 가능성이 높습니다.");
    _eightBallResponses.push_back("의심스럽습니다.");
    _eightBallResponses.push_back("네, 확실합니다.");
    _eightBallResponses.push_back("아니요, 절대 아닙니다.");
}

void Bot::_initializeWeatherData() {
    _weatherData["서울"] = "맑음, 22°C";
    _weatherData["부산"] = "흐림, 25°C";
    _weatherData["대구"] = "맑음, 28°C";
    _weatherData["인천"] = "비, 20°C";
    _weatherData["광주"] = "맑음, 26°C";
    _weatherData["대전"] = "흐림, 24°C";
    _weatherData["울산"] = "맑음, 27°C";
    _weatherData["세종"] = "맑음, 23°C";
}

void Bot::handleMessage(const std::string& sender, const std::string& channel, 
                        const std::string& message) {
    if (!_active) return;
    
    // 봇 자신의 메시지는 무시
    if (sender == _nickname) return;
    
    // 채널 메시지인지 개인 메시지인지 확인
    if (channel.empty()) {
        handlePrivateMessage(sender, message);
    } else {
        handleChannelMessage(sender, channel, message);
    }
}

void Bot::handlePrivateMessage(const std::string& sender, const std::string& message) {
    // 개인 메시지는 항상 명령어로 처리
    BotCommandType cmdType = parseCommand(message);
    if (cmdType != BOT_UNKNOWN) {
        std::vector<std::string> args = _splitString(message, ' ');
        if (!args.empty()) args.erase(args.begin()); // 명령어 제거
        executeCommand(cmdType, sender, "", args);
    } else {
        sendMessage(sender, "명령어를 인식할 수 없습니다. 'help'를 입력하세요.");
    }
}

void Bot::handleChannelMessage(const std::string& sender, const std::string& channel, 
                               const std::string& message) {
    // 봇 이름이 언급되었는지 확인
    std::string lowerMessage = message;
    for (std::string::iterator it = lowerMessage.begin(); it != lowerMessage.end(); ++it) {
        *it = tolower(*it);
    }
    
    if (lowerMessage.find(_nickname) != std::string::npos) {
        BotCommandType cmdType = parseCommand(message);
        if (cmdType != BOT_UNKNOWN) {
            std::vector<std::string> args = _splitString(message, ' ');
            if (!args.empty()) args.erase(args.begin()); // 명령어 제거
            executeCommand(cmdType, sender, channel, args);
        } else {
            sendMessage(channel, sender + ": 무엇을 도와드릴까요? 'help'를 입력하세요.");
        }
    }
}

BotCommandType Bot::parseCommand(const std::string& message) {
    std::vector<std::string> parts = _splitString(message, ' ');
    if (parts.empty()) return BOT_UNKNOWN;
    
    std::string command = parts[0];
    for (std::string::iterator it = command.begin(); it != command.end(); ++it) {
        *it = tolower(*it);
    }
    
    // 명령어 앞의 봇 이름 제거
    if (command.find(_nickname) != std::string::npos) {
        size_t pos = command.find(_nickname);
        command = command.substr(pos + _nickname.length());
        // 구두점이나 공백 제거
        while (!command.empty() && (command[0] == ':' || command[0] == ',' || command[0] == ' ')) {
            command = command.substr(1);
        }
    }
    
    std::map<std::string, BotCommandType>::iterator it = _commandMap.find(command);
    if (it != _commandMap.end()) {
        _commandCount++;
        return it->second;
    }
    
    return BOT_UNKNOWN;
}

std::vector<std::string> Bot::_splitString(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// 메시지 출력
void Bot::addOutbox(const std::string& message) {
    _outbox.push(message);
}

std::string Bot::getNextMessage() {
    if (_outbox.empty()) return "";
    
    std::string message = _outbox.front();
    _outbox.pop();
    return message;
}

bool Bot::hasMessages() const {
    return !_outbox.empty();
}

void Bot::sendMessage(const std::string& target, const std::string& message) {
    std::string formattedMessage = ":" + _nickname + " PRIVMSG " + target + " :" + message + "\r\n";
    addOutbox(formattedMessage);
}

void Bot::sendNotice(const std::string& target, const std::string& message) {
    std::string formattedMessage = ":" + _nickname + " NOTICE " + target + " :" + message + "\r\n";
    addOutbox(formattedMessage);
} 