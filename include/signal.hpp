#ifndef SIGNAL_UTILS_HPP
#define SIGNAL_UTILS_HPP
#include <csignal>

class Server;          // 전방 선언

namespace sig {
    void install(Server* svr);   // 메인에서 호출
    bool stopRequested();
}

#endif