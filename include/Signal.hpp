#ifndef SIGNAL_UTILS_HPP
#define SIGNAL_UTILS_HPP
#include <csignal>

class Server;          

namespace Sig {
    void install(Server* svr);   
    bool stopRequested();
}

#endif