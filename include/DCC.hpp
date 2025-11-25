#ifndef DCC_HPP
#define DCC_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include "SharedPtr.hpp"

class User;


class DCCSession {
public:
    enum Type {
        SEND,
        RECEIVE,
        CHAT
    };

private:
    int _id;
    Type _type;
    std::string _filename;
    std::string _sender;
    std::string _receiver;
    int _port;
    std::string _ip;
    int _socket;    
    int _clientSocket;  
    size_t _fileSize;
    size_t _bytesTransferred;
    bool _active;
    std::string _filePath;

public:
    DCCSession(int id, Type type, const std::string& filename, 
               const std::string& sender, const std::string& receiver,
               int port, const std::string& ip, size_t fileSize = 0);
    ~DCCSession();

    
    int getId() const;
    Type getType() const;
    std::string getFilename() const;
    std::string getSender() const;
    std::string getReceiver() const;
    int getPort() const;
    std::string getIP() const;
    int getSocket() const;
    int getClientSocket() const;
    size_t getFileSize() const;
    size_t getBytesTransferred() const;
    bool isActive() const;
    std::string getFilePath() const;

    
    void setSocket(int socket);
    void setClientSocket(int socket);
    void setActive(bool active);
    void setBytesTransferred(size_t bytes);
    void setFilePath(const std::string& path);

    
    bool setupConnection();
    bool sendFile();
    bool receiveFile();
    void closeConnection();
    std::string getStatusString() const;
};


class DCCManager {
private:
    std::map<int, SharedPtr<DCCSession> > _sessions;
    int _nextSessionId;
    std::string _uploadDir;
    std::vector<struct pollfd> _dccPollFds;
    std::map<int, int> _fdToSessionId; 

public:
    DCCManager();
    ~DCCManager();

    
    int createSession(DCCSession::Type type, const std::string& filename,
                     const std::string& sender, const std::string& receiver,
                     size_t fileSize = 0, const std::string& ip_addr = "", int port_num = 0);
    void removeSession(int sessionId);
    DCCSession* getSession(int sessionId);
    std::vector<DCCSession*> getSessionsByUser(const std::string& nickname);

    
    std::pair<bool, std::string> handleDCCSend(User& sender, const std::vector<std::string>& params);
    void handleDCCAccept(User& user, const std::vector<std::string>& params);
    void handleDCCResume(User& user, const std::vector<std::string>& params);
    void handleDCCReject(User& user, const std::vector<std::string>& params);

    
    void processDCCTransfers();
    bool acceptDCCConnection(int sessionId);
    void handleDCCData(int sessionId);

    
    int findAvailablePort();
    std::string getLocalIP();
    std::string getUploadDirectory() const;
    void setUploadDirectory(const std::string& dir);

private:
    void _setupUploadDirectory();
    int _createListenSocket(int port);
    void _updatePollFds();
};

#endif 