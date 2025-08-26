#include "../include/Server.hpp"
#include <iostream>

// 봇 초기화
void Server::_initializeBot() {
    // 봇 유저 생성
    User* botUser = new User(999); // 특별한 FD 번호
    botUser->setNickName(_bot.getNickname());
    botUser->setUserName(_bot.getUsername());
    botUser->setActive(true);
    
    _users.addUserWithId(botUser);
    _botUser = _users.returnSecond(botUser->getId());
    
    // 봇을 로비에 추가
    _lobby->addUser(_botUser);
    
    // 봇 초기화
    _bot.initialize();
    
    std::cout << "Bot initialized: " << _bot.getNickname() << std::endl;
}

// 봇 메시지 처리
void Server::_processBotMessages() {
    while (_bot.hasMessages()) {
        std::string message = _bot.getNextMessage();
        if (!message.empty()) {
            // 봇 메시지를 모든 채널에 브로드캐스트
            for (TotalDatabase<Channel>::it it = _channels.begin(); 
                 it != _channels.end(); ++it) {
                it->second->broadcast(message, NULL);
            }
        }
    }
}

// 봇 명령어 처리
void Server::_handleBotCommands(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 2) return;
    
    std::string target = params[0];
    std::string message = params[1];
    
    // 봇에게 보내는 메시지인지 확인
    if (target == _bot.getNickname()) {
        _bot.handleMessage(u.getNickName(), "", message);
    } else {
        // 채널 메시지인지 확인
        Channel* channel = getChannelByName(target);
        if (channel) {
            _bot.handleMessage(u.getNickName(), target, message);
        }
    }
} 