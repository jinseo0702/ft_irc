#include "../include/Server.hpp"
#include <iostream>

// DCC 명령어 핸들러들
void Server::handleDCCSend(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 3) {
        u.numeric(461, "DCC SEND :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); // receiver
    dccParams.push_back(params[1]); // filename
    dccParams.push_back(params[2]); // filesize
    
    _dccManager.handleDCCSend(u, dccParams);
}

void Server::handleDCCAccept(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 2) {
        u.numeric(461, "DCC ACCEPT :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); // filename
    dccParams.push_back(params[1]); // port
    
    _dccManager.handleDCCAccept(u, dccParams);
}

void Server::handleDCCResume(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 2) {
        u.numeric(461, "DCC RESUME :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); // filename
    dccParams.push_back(params[1]); // port
    
    _dccManager.handleDCCResume(u, dccParams);
}

void Server::handleDCCReject(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 1) {
        u.numeric(461, "DCC REJECT :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); // filename
    
    _dccManager.handleDCCReject(u, dccParams);
} 