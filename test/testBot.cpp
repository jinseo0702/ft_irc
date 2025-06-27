#include <iostream>
#include <string>
#include "../include/Bot.hpp"

int main() {
    std::cout << "=== IRC 봇 테스트 ===" << std::endl;
    
    // 봇 초기화
    Bot bot;
    bot.initialize();
    
    std::cout << "봇 정보:" << std::endl;
    std::cout << "  닉네임: " << bot.getNickname() << std::endl;
    std::cout << "  사용자명: " << bot.getUsername() << std::endl;
    std::cout << "  실명: " << bot.getRealname() << std::endl;
    std::cout << "  버전: " << bot.getVersion() << std::endl;
    std::cout << "  설명: " << bot.getDescription() << std::endl;
    
    // 명령어 테스트
    std::cout << "\n=== 명령어 테스트 ===" << std::endl;
    
    // 개인 메시지 테스트
    std::cout << "\n1. 개인 메시지 테스트:" << std::endl;
    bot.handleMessage("testuser", "", "help");
    bot.handleMessage("testuser", "", "time");
    bot.handleMessage("testuser", "", "weather 서울");
    bot.handleMessage("testuser", "", "calc 1 + 2 * 3");
    bot.handleMessage("testuser", "", "quote");
    bot.handleMessage("testuser", "", "roll 20");
    bot.handleMessage("testuser", "", "8ball 내일 비올까?");
    
    // 채널 메시지 테스트
    std::cout << "\n2. 채널 메시지 테스트:" << std::endl;
    bot.handleMessage("testuser", "#test", "ft_irc_bot: help");
    bot.handleMessage("testuser", "#test", "ft_irc_bot: time");
    bot.handleMessage("testuser", "#test", "ft_irc_bot: weather 부산");
    
    // 봇 메시지 출력
    std::cout << "\n=== 봇 응답 ===" << std::endl;
    while (bot.hasMessages()) {
        std::string message = bot.getNextMessage();
        std::cout << message;
    }
    
    // 유틸리티 함수 테스트
    std::cout << "\n=== 유틸리티 함수 테스트 ===" << std::endl;
    std::cout << "현재 시간: " << bot.getCurrentTime() << std::endl;
    std::cout << "가동 시간: " << bot.getUptime() << std::endl;
    std::cout << "랜덤 숫자 (1-10): " << bot.getRandomNumber(1, 10) << std::endl;
    std::cout << "랜덤 명언: " << bot.getRandomQuote() << std::endl;
    std::cout << "8번 공 응답: " << bot.getRandom8BallResponse() << std::endl;
    std::cout << "서울 날씨: " << bot.getWeather("서울") << std::endl;
    
    std::cout << "\n=== IRC 봇 테스트 완료 ===" << std::endl;
    return 0;
} 