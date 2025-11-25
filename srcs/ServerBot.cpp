#include "../include/Server.hpp"
#include <iostream>


void Server::_initializeBot() {
    
    User* botUser = new User(999); 
    botUser->setNickName(_bot.getNickname());
    botUser->setUserName(_bot.getUsername());
    botUser->setActive(true);
    
    _users.addUserWithId(botUser);
    _botUser = _users.returnSecond(botUser->getId());
    
    
    _lobby->addUser(_botUser);
    
    
    _bot.initialize();
    
    std::cout << "Bot initialized: " << _bot.getNickname() << std::endl;
}


void Server::_processBotMessages() {
    while (_bot.hasMessages()) {
        std::string message = _bot.getNextMessage();
        if (!message.empty()) {
            
            for (TotalDatabase<Channel>::it it = _channels.begin(); 
                 it != _channels.end(); ++it) {
                it->second->broadcast(message, NULL);
                return ;
            }
        }
    }
}


void Server::_handleBotCommands(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 2) return;
    
    std::string target = params[0];
    std::string message = params[1];
    
    
    if (target == _bot.getNickname()) {
        _bot.handleMessage(u.getNickName(), "", message);
    } else {
        
        Channel* channel = getChannelByName(target);
        if (channel) {
            _bot.handleMessage(u.getNickName(), target, message);
        }
    }
} 