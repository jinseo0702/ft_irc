#include "../include/Signal.hpp"
#include "../include/Server.hpp"

namespace {
    volatile sig_atomic_t g_stopRequested = 0;
    Server*               g_server        = 0;

    void handler(int signo)
    {
        if (signo == SIGPIPE)
            return;                 // 무시 (이미 SIG_IGN이지만 safety)
        g_stopRequested = 1;
    }
}

void Sig::install(Server* svr)
{
    g_server = svr;

    /* 1) SIGPIPE 무시 */
    std::signal(SIGPIPE, SIG_IGN);

    /* 2) SIGINT / SIGTERM 은 동일 핸들러 */
    std::signal(SIGINT,  handler);
    std::signal(SIGTERM, handler);
}

/*  run() 루프 안에서 신호 체크를 호출할 수 있도록 작은 헬퍼  */
bool Sig::stopRequested()
{
    return g_stopRequested != 0;
}