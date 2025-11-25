#include "../include/Signal.hpp"
#include "../include/Server.hpp"

namespace {
    volatile sig_atomic_t g_stopRequested = 0;
    Server*               g_server        = 0;

    void handler(int signo)
    {
        if (signo == SIGPIPE)
            return;
        g_stopRequested = 1;
    }
}

void Sig::install(Server* svr)
{
    g_server = svr;

 
    std::signal(SIGPIPE, SIG_IGN);

    std::signal(SIGINT,  handler);
    std::signal(SIGTERM, handler);
}

bool Sig::stopRequested()
{
    return g_stopRequested != 0;
}