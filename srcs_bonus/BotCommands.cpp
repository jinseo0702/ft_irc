#include "../include/Bot.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

void Bot::executeCommand(BotCommandType cmdType, const std::string& sender, 
                         const std::string& channel, const std::vector<std::string>& args) {
    std::string target = channel.empty() ? sender : channel;
    
    switch (cmdType) {
        case BOT_HELP:
            cmdHelp(sender, target, args);
            break;
        case BOT_TIME:
            cmdTime(sender, target, args);
            break;
        case BOT_WEATHER:
            cmdWeather(sender, target, args);
            break;
        case BOT_CALC:
            cmdCalc(sender, target, args);
            break;
        case BOT_QUOTE:
            cmdQuote(sender, target, args);
            break;
        case BOT_ROLL:
            cmdRoll(sender, target, args);
            break;
        case BOT_8BALL:
            cmdEightBall(sender, target, args);
            break;
        default:
            sendMessage(target, "알 수 없는 명령어입니다. 'help'를 입력하세요.");
            break;
    }
}

void Bot::cmdHelp(const std::string& sender, const std::string& target, 
                  const std::vector<std::string>& args) {

    (void)sender;
    (void)args;
    
    std::string helpMsg = "사용 가능한 명령어:\n";
    helpMsg += "  help - 이 도움말을 보여줍니다\n";
    helpMsg += "  time - 현재 시간을 보여줍니다\n";
    helpMsg += "  weather [도시] - 날씨 정보를 보여줍니다\n";
    helpMsg += "  calc [수식] - 수식을 계산합니다\n";
    helpMsg += "  quote - 랜덤 명언을 보여줍니다\n";
    helpMsg += "  roll [숫자] - 주사위를 굴립니다\n";
    helpMsg += "  8ball [질문] - 8번 공에게 질문합니다\n";
    
    sendMessage(target, helpMsg);
}

void Bot::cmdTime(const std::string& sender, const std::string& target, 
                  const std::vector<std::string>& args) {

    (void)sender;
    (void)args;
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y년 %m월 %d일 %H시 %M분 %S초", timeinfo);
    
    std::string msg = "현재 시간: ";
    msg += timeStr;
    
    sendMessage(target, msg);
}

void Bot::cmdWeather(const std::string& sender, const std::string& target, 
                     const std::vector<std::string>& args) {
                        
                        
    (void)sender;
    std::string city = "서울"; // 기본값
    
    if (!args.empty()) {
        city = args[0];
    }
    
    std::map<std::string, std::string>::iterator it = _weatherData.find(city);
    if (it != _weatherData.end()) {
        std::string msg = city + " 날씨: " + it->second;
        sendMessage(target, msg);
    } else {
        std::string msg = city + "의 날씨 정보를 찾을 수 없습니다.";
        sendMessage(target, msg);
    }
}

void Bot::cmdCalc(const std::string& sender, const std::string& target, 
                  const std::vector<std::string>& args) {

    (void)sender;
    if (args.empty()) {
        sendMessage(target, "계산할 수식을 입력하세요. 예: calc 2+3*4");
        return;
    }
    
    std::string expression = args[0];
    double result = evaluateExpression(expression);
    
    std::ostringstream oss;
    oss << expression << " = " << result;
    sendMessage(target, oss.str());
}

void Bot::cmdQuote(const std::string& sender, const std::string& target, 
                   const std::vector<std::string>& args) {
    
    (void)sender;
    (void)args;
    if (_quotes.empty()) {
        sendMessage(target, "명언이 없습니다.");
        return;
    }
    
    int index = getRandomNumber(0, _quotes.size() - 1);
    sendMessage(target, _quotes[index]);
}

void Bot::cmdRoll(const std::string& sender, const std::string& target, 
                  const std::vector<std::string>& args) {
    int max = 6; // 기본값

    (void)sender;
    if (!args.empty()) {
        std::istringstream iss(args[0]);
        iss >> max;
        if (max <= 0) max = 6;
    }
    
    int result = getRandomNumber(1, max);
    std::ostringstream oss;
    oss << "주사위 결과: " << result << " (1-" << max << ")";
    sendMessage(target, oss.str());
}

void Bot::cmdEightBall(const std::string& sender, const std::string& target, 
                       const std::vector<std::string>& args) {

    (void)sender;
    if (args.empty()) {
        sendMessage(target, "질문을 입력하세요. 예: 8ball 내일 비가 올까?");
        return;
    }
    
    if (_eightBallResponses.empty()) {
        sendMessage(target, "8번 공이 대답할 수 없습니다.");
        return;
    }
    
    int index = getRandomNumber(0, _eightBallResponses.size() - 1);
    sendMessage(target, _eightBallResponses[index]);
}

double Bot::evaluateExpression(const std::string& expression) const {
    // 간단한 수식 계산 (C++98 호환)
    std::istringstream iss(expression);
    double result = 0.0;
    char op = '+';
    double num;
    
    while (iss >> num) {
        switch (op) {
            case '+': result += num; break;
            case '-': result -= num; break;
            case '*': result *= num; break;
            case '/': if (num != 0) result /= num; break;
        }
        iss >> op;
    }
    
    return result;
}

int Bot::getRandomNumber(int min, int max) const {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    
    return min + (rand() % (max - min + 1));
} 