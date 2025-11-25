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


DCCManager::DCCManager() : _nextSessionId(1) {
    _setupUploadDirectory();
}

DCCManager::~DCCManager() {
    
    for (std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.begin();
         it != _sessions.end(); ++it) {
        it->second->closeConnection();
    }
    _sessions.clear();
}

void DCCManager::_setupUploadDirectory() {
    _uploadDir = "./uploads";
    if (mkdir(_uploadDir.c_str(), 0755) == 0)
        std::cout << "PLZ Create directory file name 'uploads'!!" << std::endl;
}

int DCCManager::createSession(DCCSession::Type type, const std::string& filename,
                             const std::string& sender, const std::string& receiver,
                             size_t fileSize, const std::string& ip_addr, int port_num) {
    int port = (port_num == 0) ? findAvailablePort() : port_num;
    std::string ip = (ip_addr.empty()) ? getLocalIP() : ip_addr;
        
    if (port == 0) {
        std::cerr << "DCC Error: No available ports for sending." << std::endl;
        return -1;
    }
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
    else {
        std::cout << "DCC: setupConnection failed for session " << _nextSessionId << std::endl;
        return -1;
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
    std::vector<int> sessions_to_remove;

    for (std::map<int, SharedPtr<DCCSession> >::iterator it = _sessions.begin();
         it != _sessions.end(); ++it) {
        DCCSession* session = it->second.get();
        if (!session || !session->isActive()) {
            continue;
        }

        if (session->getType() == DCCSession::SEND) {
            if (session->getClientSocket() < 0) {
                int clientSocket = accept(session->getSocket(), NULL, NULL);

                if (clientSocket >= 0) {
                    std::cout << "DCC (Send): Connection accepted for session " << session->getId() << std::endl;
                    session->setClientSocket(clientSocket);
                    fcntl(clientSocket, F_SETFL, O_NONBLOCK);
                    close(session->getSocket());
                    session->setSocket(-1);

                } else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("DCC (Send) accept error");
                        sessions_to_remove.push_back(session->getId());
                    }
                }
            }
            else {
                session->sendFile();
            }
        } else if (session->getType() == DCCSession::RECEIVE) {
            session->receiveFile();
        }
        if (!session->isActive()) {
            sessions_to_remove.push_back(session->getId());
        }
    }

    for (size_t i = 0; i < sessions_to_remove.size(); ++i) {
        removeSession(sessions_to_remove[i]);
    }
}

std::pair<bool, std::string> DCCManager::handleDCCSend(User& user, const std::vector<std::string>& params) {
    if (params.size() < 3) {
        return std::make_pair(false, ":server 461 " + user.getNickName() + " DCC SEND :Not enough parameters\r\n");
    }

    std::string receiverNick = params[0];
    std::string filename = params[1];
    std::string fileSizeStr = params[2];

    size_t fileSize = 0;
    std::istringstream iss(fileSizeStr);
    iss >> fileSize;

    if (fileSize == 0) {
        return std::make_pair(false, ":server 461 " + user.getNickName() + " DCC SEND :Invalid file size\r\n");
    }

    int port = findAvailablePort();
    std::string ip = user.getHostname();
    if (ip == "127.0.0.1") {
        ip = user.getServerAddress();
    }
    int sessionId = createSession(DCCSession::SEND, filename, user.getNickName(), receiverNick, fileSize, ip, port);

    if (sessionId > 0) {
        DCCSession* session = getSession(sessionId);
        if (session) {
            std::ostringstream oss;
            oss << ":" << user.fullPrefix()
                << " PRIVMSG " << receiverNick
                << " :\001DCC SEND " << filename << " " << session->getIP() 
                << " " << session->getPort() << " " << fileSize << "\001\r\n";

            
            return std::make_pair(true, oss.str());
        }
    }
    return std::make_pair(false, ":server 461 " + user.getNickName() + " DCC :Failed to create DCC session\r\n");
}

void DCCManager::handleDCCAccept(User& user, const std::vector<std::string>& params) {
    if (params.size() < 5) {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC ACCEPT :Not enough parameters\r\n");
        return;
    }

    std::string filename = params[0];
    std::string ip = params[1];
    std::string portStr = params[2];
    std::string fileSizeStr = params[3];
    std::string senderNick = params[4];

    int port = 0;
    std::istringstream port_iss(portStr);
    port_iss >> port;

    size_t fileSize = 0;
    std::istringstream size_iss(fileSizeStr);
    size_iss >> fileSize;

    if (port <= 0 || port > 65535 || fileSize == 0) {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC ACCEPT :Invalid parameters\r\n");
        return;
    }

    
    int sessionId = createSession(DCCSession::RECEIVE, filename, senderNick, user.getNickName(),
                                 fileSize, ip, port);

    if (sessionId > 0) {
        std::cout << "DCC ACCEPT session created: " << sessionId
                  << ". Connecting to " << senderNick << "..." << std::endl;
    } else {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC :Failed to create DCC session for receiving\r\n");
    }
}

void DCCManager::handleDCCResume(User& user, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC :Not enough parameters\r\n");
        return;
    }
    
    std::string filename = params[0];
    std::string portStr = params[1];
    
    
    int port = 0;
    std::istringstream iss(portStr);
    iss >> port;
    
    if (port <= 0 || port > 65535) {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC :Invalid port number\r\n");
        return;
    }
    
    
    std::ostringstream oss;
    oss << ":" << user.getNickName() << " PRIVMSG " << user.getNickName() 
        << " :\001DCC RESUME " << filename << " " << port << "\001\r\n";
    user.addOutbox(oss.str());
    
    std::cout << "DCC RESUME requested for: " << filename << std::endl;
}

void DCCManager::handleDCCReject(User& user, const std::vector<std::string>& params) {
    if (params.size() < 1) {
        user.addOutbox(":server 461 " + user.getNickName() + " DCC :Not enough parameters\r\n");
        return;
    }
    
    std::string filename = params[0];
    
    
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
    
    return 0; 
}

std::string DCCManager::getLocalIP() {
    
    return "127.0.0.1";
}

std::string DCCManager::getUploadDirectory() const {
    return _uploadDir;
}

void DCCManager::setUploadDirectory(const std::string& dir) {
    _uploadDir = dir;
} 