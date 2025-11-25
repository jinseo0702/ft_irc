#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <ctime>
#include "User.hpp"
#include "SharedPtr.hpp"

class Channel;
class Server;

enum BotCommandType {
    BOT_HELP,
    BOT_TIME,
    BOT_WEATHER,
    BOT_CALC,
    BOT_QUOTE,
    BOT_ROLL,
    BOT_8BALL,
    BOT_UNKNOWN
};

struct BotCommand {
    BotCommandType type;
    std::string command;
    std::string description;
    std::vector<std::string> aliases;
};

class Bot {
private:
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _version;
    std::string _description;
    bool _active;
    std::queue<std::string> _outbox;
    
    std::map<std::string, BotCommandType> _commandMap;
    std::vector<BotCommand> _commands;
    
    std::vector<std::string> _quotes;
    std::vector<std::string> _eightBallResponses;
    std::map<std::string, std::string> _weatherData;
    
    int _commandCount;
    time_t _startTime;

public:
    Bot();
    ~Bot();
    
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& real);
    void setVersion(const std::string& ver);
    void setDescription(const std::string& desc);
    void setActive(bool active);
    
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getVersion() const;
    std::string getDescription() const;
    bool isActive() const;
    std::queue<std::string>& getOutbox();
    
    void initialize();
    void setupCommands();
    void loadQuotes();
    void loadEightBallResponses();
    void loadWeatherData();
    
    void handleMessage(const std::string& sender, const std::string& channel, 
                      const std::string& message);
    void handlePrivateMessage(const std::string& sender, const std::string& message);
    void handleChannelMessage(const std::string& sender, const std::string& channel, 
                             const std::string& message);
    
    BotCommandType parseCommand(const std::string& message);
    void executeCommand(BotCommandType cmdType, const std::string& sender, 
                       const std::string& channel, const std::vector<std::string>& args);
    
    void cmdHelp(const std::string& sender, const std::string& channel, 
                 const std::vector<std::string>& args);
    void cmdTime(const std::string& sender, const std::string& channel, 
                 const std::vector<std::string>& args);
    void cmdWeather(const std::string& sender, const std::string& channel, 
                    const std::vector<std::string>& args);
    void cmdCalc(const std::string& sender, const std::string& channel, 
                 const std::vector<std::string>& args);
    void cmdQuote(const std::string& sender, const std::string& channel, 
                  const std::vector<std::string>& args);
    void cmdRoll(const std::string& sender, const std::string& channel, 
                 const std::vector<std::string>& args);
    void cmdEightBall(const std::string& sender, const std::string& channel, 
                      const std::vector<std::string>& args);
    void cmdStats(const std::string& sender, const std::string& channel, 
                  const std::vector<std::string>& args);
    void cmdVersion(const std::string& sender, const std::string& channel, 
                    const std::vector<std::string>& args);
    
    void sendMessage(const std::string& target, const std::string& message);
    void sendNotice(const std::string& target, const std::string& message);
    std::string getCurrentTime() const;
    std::string getUptime() const;
    int getRandomNumber(int min, int max) const;
    std::string getRandomQuote() const;
    std::string getRandom8BallResponse() const;
    std::string getWeather(const std::string& city) const;
    double evaluateExpression(const std::string& expression) const;
    
    void addOutbox(const std::string& message);
    std::string getNextMessage();
    bool hasMessages() const;
    
private:
    void _initializeDefaultQuotes();
    void _initializeEightBallResponses();
    void _initializeWeatherData();
    std::vector<std::string> _splitString(const std::string& str, char delimiter) const;
    bool _isValidExpression(const std::string& expr) const;
};

#endif 