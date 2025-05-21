#include "../include/server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>

/* ───────── 생성자 ───────── */
Server::Server(int port) : _listenFd(-1), _lobby()
{
    _setupSocket(port);
    std::cout << "Listening on port " << port
              << " | 모든 클라이언트는 자동으로 #lobby 입장" << std::endl;
}

/* ───────── 소켓 준비 ────── */
void Server::_setupSocket(int port)
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        perror("socket");
        std::exit(1);
    }
    int yes = 1;
    setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in a; std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(port);

    if (bind(_listenFd, (struct sockaddr*)&a, sizeof(a)) < 0)
    {
        perror("bind");
        std::exit(1);
    }
    if (listen(_listenFd, 20) < 0)
    {
        perror("listen");
        std::exit(1);
    }
    fcntl(_listenFd, F_SETFL, O_NONBLOCK);
    struct pollfd pfd;
    pfd.fd = _listenFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pfds.push_back(pfd);
}

/* ───────── 메인 루프 ────── */
void Server::run()
{
    while (true)
    {
        if (poll(&_pfds[0], _pfds.size(), -1) < 0)
            perror("poll"); break;

        /* (A) 새 연결인지? */
        if (_pfds[0].revents & POLLIN)
            _acceptClient();

        /* (B) 손님별 I/O 처리하기 */
        size_t i = 1;
        while (i < _pfds.size())
        {
            int fd = _pfds[i].fd;
            if (_pfds[i].revents & POLLIN)
                _readLines(_users[fd], i);
            if (i < _pfds.size() && (_pfds[i].revents & POLLOUT))
                _flushOut (_users[fd], i);
            ++i;
        }
    }
}

/* ───────── accept ──────── */
void Server::_acceptClient()
{
    int cfd = accept(_listenFd, 0, 0);
    if (cfd < 0)
        return;
    fcntl(cfd, F_SETFL, O_NONBLOCK);

    struct pollfd pfd;
    pfd.fd = cfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pfds.push_back(pfd);

    std::map<int, User>::iterator it = _users.insert(std::make_pair(cfd, User(cfd))).first;
    User& u = it->second;
    _lobby.users.insert(&u);

    std::cout << " + client fd=" << cfd << " joined #lobby" << std::endl;
    u.outbox.push(":server NOTICE * :Welcome to #lobby\r\n");
    _pfds.back().events |= POLLOUT;          // 바로 송신 시도
}

/* ───────── 읽기 + 줄 분할 ─  나중에 여기서 파싱한 명령어를 처리하도록 만들기 */
void Server::_readLines(User& u, size_t idx)
{
    char buf[512];
    ssize_t n = recv(u.fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) // EOF or error → 정리
    {
        std::cout << " - client fd=" << u.fd << " quit" << std::endl;
        close(u.fd);
        _lobby.users.erase(&u);
        _users.erase(u.fd);
        _pfds.erase(_pfds.begin() + idx);
        return;
    }
    buf[n] = '\0';
    u.ibuf.append(buf, n);

    size_t pos;
    while ((pos = u.ibuf.find('\n')) != std::string::npos)
    {
        std::string line = u.ibuf.substr(0, pos);
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size()-1, 1);
        u.ibuf.erase(0, pos + 1);

        /* “명령어” 따로 없음 – 파싱 이후에 집어 넣을 것 */
        std::string msg = ":" + _fdToStr(u.fd)
                        + " PRIVMSG #lobby :" + line + "\r\n";
        _lobby.broadcast(msg, &u);
    }
}

/* ───────── 송신 버퍼 비우기 ─ */
void Server::_flushOut(User& u, size_t idx)
{
    while (!u.outbox.empty())
    {
        const std::string& m = u.outbox.front();
        ssize_t n = send(u.fd, m.c_str(), m.size(), 0);
        if (n == (ssize_t)m.size())
            u.outbox.pop();
        else
            break;               // 다 못 보냈으면 다음 POLLOUT 때 재도전하기
    }
    if (u.outbox.empty())
        _pfds[idx].events &= ~POLLOUT;  // 대기 해제
}

/* ────── fd → string  ────── */
std::string Server::_fdToStr(int fd) const
{
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}
