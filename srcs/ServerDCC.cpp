#include "../include/Server.hpp"
#include <iostream>


void Server::handleDCCSend(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    
    if (params.size() < 3) {
        u.numeric(461, "DCC SEND :Not enough parameters. Usage: DCC SEND <nick> <file> <size>");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); 
    dccParams.push_back(params[1]); 
    dccParams.push_back(params[2]); 
    
    _dccManager.handleDCCSend(u, dccParams);
}

void Server::handleDCCAccept(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    
    if (params.size() < 5) {
        u.numeric(461, "DCC ACCEPT :Not enough parameters. Usage: DCC ACCEPT <file> <ip> <port> <size> <sender>");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); 
    dccParams.push_back(params[1]); 
    dccParams.push_back(params[2]); 
    dccParams.push_back(params[3]); 
    dccParams.push_back(params[4]); 
    
    _dccManager.handleDCCAccept(u, dccParams);
}

void Server::handleDCCResume(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 2) {
        u.numeric(461, "DCC RESUME :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); 
    dccParams.push_back(params[1]); 
    
    _dccManager.handleDCCResume(u, dccParams);
}

void Server::handleDCCReject(User& u, const Parser& p) {
    const std::vector<std::string>& params = p.getParams();
    if (params.size() < 1) {
        u.numeric(461, "DCC REJECT :Not enough parameters");
        return;
    }
    
    std::vector<std::string> dccParams;
    dccParams.push_back(params[0]); 
    
    _dccManager.handleDCCReject(u, dccParams);
} 