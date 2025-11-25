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
#include <arpa/inet.h>
#include <cerrno>



DCCSession::DCCSession(int id, Type type, const std::string& filename, 
                       const std::string& sender, const std::string& receiver,
                       int port, const std::string& ip, size_t fileSize)
    : _id(id), _type(type), _filename(filename), _sender(sender), 
      _receiver(receiver), _port(port), _ip(ip), _socket(-1), _clientSocket(-1),
      _fileSize(fileSize), _bytesTransferred(0), _active(false)
{
}

DCCSession::~DCCSession() {
    closeConnection();
}

bool DCCSession::setupConnection() {
    if (_type == SEND) {
        
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket < 0) {
            std::cerr << "DCC: Failed to create listening socket" << std::endl;
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
            std::cerr << "DCC: Failed to bind socket on port " << _port << std::endl;
            perror("DCC bind");
            close(_socket);
            _socket = -1;
            return false;
        }
        
        if (listen(_socket, 1) < 0) {
            std::cerr << "DCC: Failed to listen on socket" << std::endl;
            perror("DCC listen");
            close(_socket);
            _socket = -1;
            return false;
        }

        
        fcntl(_socket, F_SETFL, O_NONBLOCK);
        _active = true; 
        std::cout << "DCC (Send): Listening on port " << _port << " for file " << _filename << std::endl;

    } else if (_type == RECEIVE) {
        
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket < 0) {
            std::cerr << "DCC (Receive): Failed to create socket" << std::endl;
            return false;
        }

        struct sockaddr_in remote_addr;
        std::memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(_port); 

        if (inet_pton(AF_INET, _ip.c_str(), &remote_addr.sin_addr) <= 0) {
            std::cerr << "DCC (Receive): Invalid IP address provided by sender" << std::endl;
            close(_socket);
            _socket = -1;
            return false;
        }

        
        fcntl(_socket, F_SETFL, O_NONBLOCK);

        if (connect(_socket, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0) {
            
            if (errno != EINPROGRESS) {
                perror("DCC (Receive) connect error");
                close(_socket);
                _socket = -1;
                return false;
            }
        }
        
        _active = true;
        std::cout << "DCC (Receive): Attempting to connect to " << _ip << ":" << _port << " for file " << _filename << std::endl;
    }
    return true;
}

bool DCCSession::receiveFile() {
    if (_socket < 0 || !_active) return false;

    char buffer[4096];
    ssize_t received = recv(_socket, buffer, sizeof(buffer), 0);

    if (received > 0) {
        std::ofstream file(_filePath.c_str(), std::ios::binary | std::ios::app);
        if (file.is_open()) {
            file.write(buffer, received);
            file.close();
            _bytesTransferred += received;
            std::cout << "DCC (Receive): " << _bytesTransferred << " / " << _fileSize << " bytes received." << std::endl;
        } else {
            std::cerr << "DCC (Receive): Failed to open file for writing: " << _filePath << std::endl;
            _active = false; 
            return false;
        }

        uint32_t ack = htonl(_bytesTransferred);
        send(_socket, &ack, sizeof(ack), 0);

        if (_fileSize > 0 && _bytesTransferred >= _fileSize) {
            std::cout << "DCC (Receive): File transfer completed for " << _filename << std::endl;
            _active = false;
            closeConnection();
        }
    } else if (received == 0) {
        std::cout << "DCC (Receive): Connection closed by sender." << std::endl;
        _active = false;
        closeConnection();
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("DCC (Receive) recv error");
            _active = false;
            closeConnection();
        }
    }

    return true;
}

bool DCCSession::sendFile() {
    
    if (_clientSocket < 0 || !_active) return false;

    std::ifstream file(_filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "DCC: Failed to open file for sending: " << _filePath << std::endl;
        _active = false; 
        return false;
    }

    
    file.seekg(_bytesTransferred, std::ios::beg);

    char buffer[4096];
    file.read(buffer, sizeof(buffer));
    
    size_t bytesRead = file.gcount();

    if (bytesRead > 0) {
        ssize_t sent = send(_clientSocket, buffer, bytesRead, 0);
        if (sent > 0) {
            _bytesTransferred += sent;
            std::cout << "DCC (Send): " << _bytesTransferred << " / " << _fileSize << " bytes sent." << std::endl;

            
            
            
        } else if (sent < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                 perror("DCC (Send) send error");
                 _active = false;
                 closeConnection();
            }
        }
    }

    file.close();

    
    if (_bytesTransferred >= _fileSize) {
        std::cout << "DCC: File transfer completed for " << _filename << std::endl;
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
    if (_clientSocket >= 0) {
        close(_clientSocket);
        _clientSocket = -1;
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
DCCSession::Type DCCSession::getType() const { return _type; }
bool DCCSession::isActive() const { return _active; }
int DCCSession::getSocket() const { return _socket; }
int DCCSession::getClientSocket() const { return _clientSocket; }
void DCCSession::setClientSocket(int socket) { _clientSocket = socket; }
void DCCSession::setFilePath(const std::string& path) { _filePath = path; }
void DCCSession::setActive(bool active) { _active = active; }
void DCCSession::setSocket(int socket) { _socket = socket; }
int DCCSession::getId() const {
    return _id;
}