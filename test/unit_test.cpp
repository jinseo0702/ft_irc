#include <iostream>
#include <string>
#include <cassert>
#include "../include/User.hpp"
#include "../include/Bot.hpp"

// 간단한 단위 테스트 함수들
void testUser() {
    std::cout << "=== User 클래스 테스트 ===" << std::endl;
    
    User user(123);
    assert(user.getFd() == 123);
    assert(user.getId() != -1);
    assert(user.getNickName().empty());
    assert(user.getUserName().empty());
    
    user.setNickName("testuser");
    user.setUserName("testuser");
    user.setActive(true);
    
    assert(user.getNickName() == "testuser");
    assert(user.getUserName() == "testuser");
    assert(user.getActive() == true);
    
    std::cout << "✓ User 클래스 테스트 통과" << std::endl;
}

void testBot() {
    std::cout << "=== Bot 클래스 테스트 ===" << std::endl;
    
    Bot bot;
    bot.initialize();
    
    assert(bot.getNickname() == "ft_irc_bot");
    assert(bot.getUsername() == "bot");
    assert(bot.isActive() == true);
    
    // 명령어 테스트
    bot.handleMessage("testuser", "", "help");
    assert(bot.hasMessages());
    
    std::string response = bot.getNextMessage();
    assert(!response.empty());
    assert(response.find("ft_irc_bot") != std::string::npos);
    
    std::cout << "✓ Bot 클래스 테스트 통과" << std::endl;
}

void testCpp98Compatibility() {
    std::cout << "=== C++98 호환성 테스트 ===" << std::endl;
    
    // C++98 스타일 코드 테스트
    std::string str = "test";
    std::string::iterator it = str.begin();
    for (; it != str.end(); ++it) {
        *it = toupper(*it);
    }
    assert(str == "TEST");
    
    // NULL 사용 테스트
    void* ptr = NULL;
    assert(ptr == 0);
    
    std::cout << "✓ C++98 호환성 테스트 통과" << std::endl;
}

int main() {
    std::cout << "=== ft_irc 단위 테스트 시작 ===" << std::endl;
    
    try {
        testUser();
        testBot();
        testCpp98Compatibility();
        
        std::cout << "\n🎉 모든 단위 테스트 통과!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ 테스트 실패: " << e.what() << std::endl;
        return 1;
    }
} 