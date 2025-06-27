#include "../include/DCC.hpp"
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

// DCCSession 구현
DCCSession::DCCSession(int id, Type type, const std::string& filename, 
                       const std::string& sender, const std::string& receiver,
                       int port, const std::string& ip, size_t fileSize)
    : _id(id), _type(type), _filename(filename), _sender(sender), 
      _receiver(receiver), _port(port), _ip(ip), _socket(-1), 
      _fileSize(fileSize), _bytesTransferred(0), _active(false)
{
}

DCCSession::~DCCSession() {
    closeConnection();
}

bool DCCSession::setupConnection() {
    if (_type == SEND) {
        // 파일 전송을 위한 리스닝 소켓 생성
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket < 0) {
            std::cerr << "DCC: Failed to create socket" << std::endl;
            return false;
        }

        int yes = 1;
        setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(_port);

        if (bind(_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "DCC: Failed to bind socket" << std::endl;
            close(_socket);
            _socket = -1;
            return false;
        }

        if (listen(_socket, 1) < 0) {
            std::cerr << "DCC: Failed to listen" << std::endl;
            close(_socket);
            _socket = -1;
            return false;
        }

        fcntl(_socket, F_SETFL, O_NONBLOCK);
        _active = true;
    }
    return true;
}

bool DCCSession::sendFile() {
    if (_socket < 0 || !_active) return false;

    // 파일 열기
    std::ifstream file(_filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "DCC: Failed to open file: " << _filePath << std::endl;
        return false;
    }

    // 파일 크기 확인
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(_bytesTransferred, std::ios::beg);

    // 청크 단위로 전송
    char buffer[4096];
    size_t bytesToSend = (sizeof(buffer) < (fileSize - _bytesTransferred)) ? 
                         sizeof(buffer) : (fileSize - _bytesTransferred);
    
    file.read(buffer, bytesToSend);
    size_t bytesRead = file.gcount();

    if (bytesRead > 0) {
        ssize_t sent = send(_socket, buffer, bytesRead, 0);
        if (sent > 0) {
            _bytesTransferred += sent;
            
            // 진행률 업데이트 (ACK 전송)
            uint32_t ack = htonl(_bytesTransferred);
            send(_socket, &ack, sizeof(ack), 0);
        }
    }

    file.close();

    // 전송 완료 확인
    if (_bytesTransferred >= fileSize) {
        _active = false;
        closeConnection();
        return true;
    }

    return true;
}

bool DCCSession::receiveFile() {
    if (_socket < 0 || !_active) return false;

    char buffer[4096];
    ssize_t received = recv(_socket, buffer, sizeof(buffer), 0);
    
    if (received > 0) {
        // 파일에 데이터 쓰기
        std::ofstream file(_filePath.c_str(), std::ios::binary | std::ios::app);
        if (file.is_open()) {
            file.write(buffer, received);
            file.close();
            _bytesTransferred += received;
        }

        // ACK 전송
        uint32_t ack = htonl(_bytesTransferred);
        send(_socket, &ack, sizeof(ack), 0);

        // 전송 완료 확인
        if (_bytesTransferred >= _fileSize) {
            _active = false;
            closeConnection();
            return true;
        }
    } else if (received == 0) {
        // 연결 종료
        _active = false;
        closeConnection();
    }

    return true;
}

void DCCSession::closeConnection() {
    if (_socket >= 0) {
        close(_socket);
        _socket = -1;
    }
    _active = false;
}

std::string DCCSession::getStatusString() const {
    std::ostringstream oss;
    oss << "DCC " << (_type == SEND ? "SEND" : "RECEIVE") 
        << " " << _filename << " (" << _bytesTransferred 
        << "/" << _fileSize << " bytes)";
    return oss.str();
}

std::string DCCSession::getIP() const { return _ip; }
int DCCSession::getPort() const { return _port; }
void DCCSession::setFilePath(const std::string& path) { _filePath = path; }
void DCCSession::setActive(bool active) { _active = active; }
void DCCSession::setSocket(int socket) { _socket = socket; }
DCCSession::Type DCCSession::getType() const { return _type; }
bool DCCSession::isActive() const { return _active; }
int DCCSession::getSocket() const { return _socket; } 