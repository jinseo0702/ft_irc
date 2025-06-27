#include "../include/DCC.hpp"
#include "../include/User.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <cstdlib>

// DCCManager 구현
DCCManager::DCCManager() : _nextSessionId(1) {
    _setupUploadDirectory();
}

DCCManager::~DCCManager() {
    // 모든 세션 정리
    for (std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.begin();
         it != _sessions.end(); ++it) {
        it->second->closeConnection();
    }
    _sessions.clear();
}

void DCCManager::_setupUploadDirectory() {
    _uploadDir = "./uploads";
    mkdir(_uploadDir.c_str(), 0755);
}

int DCCManager::createSession(DCCSession::Type type, const std::string& filename,
                             const std::string& sender, const std::string& receiver,
                             size_t fileSize) {
    int port = findAvailablePort();
    std::string ip = getLocalIP();
    
    SharedPtr<DCCSession> session(new DCCSession(_nextSessionId, type, filename,
                                                sender, receiver, port, ip, fileSize));
    
    if (type == DCCSession::RECEIVE) {
        std::string filePath = _uploadDir + "/" + filename;
        session->setFilePath(filePath);
    } else if (type == DCCSession::SEND) {
        session->setFilePath(filename);
    }

    if (session->setupConnection()) {
        _sessions[_nextSessionId] = session;
        return _nextSessionId++;
    }
    
    return -1;
}

void DCCManager::removeSession(int sessionId) {
    std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.find(sessionId);
    if (it != _sessions.end()) {
        it->second->closeConnection();
        _sessions.erase(it);
    }
}

DCCSession* DCCManager::getSession(int sessionId) {
    std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.find(sessionId);
    if (it != _sessions.end()) {
        return it->second.get();
    }
    return NULL;
}

void DCCManager::processDCCTransfers() {
    for (std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.begin();
         it != _sessions.end(); ++it) {
        DCCSession* session = it->second.get();
        if (session && session->isActive()) {
            if (session->getType() == DCCSession::SEND) {
                session->sendFile();
            } else if (session->getType() == DCCSession::RECEIVE) {
                session->receiveFile();
            }
        }
    }
}

void DCCManager::handleDCCSend(User& user, const std::vector<std::string>& params) {
    if (params.size() < 3) {
        user.addOutbox(":server ERROR :DCC SEND requires receiver, filename, and filesize\r\n");
        return;
    }
    
    std::string receiver = params[0];
    std::string filename = params[1];
    std::string fileSizeStr = params[2];
    
    // 파일 크기 파싱
    size_t fileSize = 0;
    std::istringstream iss(fileSizeStr);
    iss >> fileSize;
    
    if (fileSize == 0) {
        user.addOutbox(":server ERROR :Invalid file size\r\n");
        return;
    }
    
    // DCC 세션 생성
    int sessionId = createSession(DCCSession::SEND, filename, 
                                 user.getNickName(), receiver, fileSize);
    
    if (sessionId > 0) {
        DCCSession* session = getSession(sessionId);
        if (session) {
            // DCC SEND 메시지 전송
            std::ostringstream oss;
            oss << ":" << user.getNickName() << " PRIVMSG " << receiver 
                << " :\001DCC SEND " << filename << " " << session->getIP() 
                << " " << session->getPort() << " " << fileSize << "\001\r\n";
            user.addOutbox(oss.str());
            
            std::cout << "DCC SEND session created: " << sessionId << std::endl;
        }
    } else {
        user.addOutbox(":server ERROR :Failed to create DCC session\r\n");
    }
}

void DCCManager::handleDCCAccept(User& user, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        user.addOutbox(":server ERROR :DCC ACCEPT requires filename and port\r\n");
        return;
    }
    
    std::string filename = params[0];
    std::string portStr = params[1];
    
    // 포트 파싱
    int port = 0;
    std::istringstream iss(portStr);
    iss >> port;
    
    if (port <= 0 || port > 65535) {
        user.addOutbox(":server ERROR :Invalid port number\r\n");
        return;
    }
    
    // DCC 세션 생성 (수신용)
    int sessionId = createSession(DCCSession::RECEIVE, filename, 
                                 user.getNickName(), "", 0);
    
    if (sessionId > 0) {
        DCCSession* session = getSession(sessionId);
        if (session) {
            // DCC ACCEPT 메시지 전송
            std::ostringstream oss;
            oss << ":" << user.getNickName() << " PRIVMSG " << user.getNickName() 
                << " :\001DCC ACCEPT " << filename << " " << port << "\001\r\n";
            user.addOutbox(oss.str());
            
            std::cout << "DCC ACCEPT session created: " << sessionId << std::endl;
        }
    } else {
        user.addOutbox(":server ERROR :Failed to create DCC session\r\n");
    }
}

void DCCManager::handleDCCResume(User& user, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        user.addOutbox(":server ERROR :DCC RESUME requires filename and port\r\n");
        return;
    }
    
    std::string filename = params[0];
    std::string portStr = params[1];
    
    // 포트 파싱
    int port = 0;
    std::istringstream iss(portStr);
    iss >> port;
    
    if (port <= 0 || port > 65535) {
        user.addOutbox(":server ERROR :Invalid port number\r\n");
        return;
    }
    
    // DCC RESUME 메시지 전송
    std::ostringstream oss;
    oss << ":" << user.getNickName() << " PRIVMSG " << user.getNickName() 
        << " :\001DCC RESUME " << filename << " " << port << "\001\r\n";
    user.addOutbox(oss.str());
    
    std::cout << "DCC RESUME requested for: " << filename << std::endl;
}

void DCCManager::handleDCCReject(User& user, const std::vector<std::string>& params) {
    if (params.size() < 1) {
        user.addOutbox(":server ERROR :DCC REJECT requires filename\r\n");
        return;
    }
    
    std::string filename = params[0];
    
    // DCC REJECT 메시지 전송
    std::ostringstream oss;
    oss << ":" << user.getNickName() << " PRIVMSG " << user.getNickName() 
        << " :\001DCC REJECT " << filename << "\001\r\n";
    user.addOutbox(oss.str());
    
    std::cout << "DCC REJECT requested for: " << filename << std::endl;
}

bool DCCManager::acceptDCCConnection(int sessionId) {
    DCCSession* session = getSession(sessionId);
    if (!session || session->getType() != DCCSession::SEND) return false;
    
    socklen_t clientLen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
    std::memset(&clientAddr, 0, clientLen);
    
    int clientSocket = accept(session->getSocket(), 
                             (struct sockaddr*)&clientAddr, &clientLen);
    
    if (clientSocket >= 0) {
        session->setSocket(clientSocket);
        session->setActive(true);
        return true;
    }
    
    return false;
}

void DCCManager::handleDCCData(int sessionId) {
    DCCSession* session = getSession(sessionId);
    if (!session || !session->isActive()) return;
    
    if (session->getType() == DCCSession::SEND) {
        session->sendFile();
    } else if (session->getType() == DCCSession::RECEIVE) {
        session->receiveFile();
    }
}

int DCCManager::findAvailablePort() {
    // 간단한 포트 찾기 (1024-65535 범위에서)
    for (int port = 1024; port <= 65535; port++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        int yes = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            close(sock);
            return port;
        }
        
        close(sock);
    }
    
    return 0; // 사용 가능한 포트가 없음
}

std::string DCCManager::getLocalIP() {
    // 로컬 IP 주소 가져오기 (간단한 구현)
    return "127.0.0.1";
}

std::string DCCManager::getUploadDirectory() const {
    return _uploadDir;
}

void DCCManager::setUploadDirectory(const std::string& dir) {
    _uploadDir = dir;
} 