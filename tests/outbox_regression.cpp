#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/TotalDatabase.hpp"
#include "../include/SharedPtr.hpp"
#include "../include/Password.hpp"
#include "../include/Parser.hpp"
#include "../include/Rulehandle.hpp"
#include "../include/Bot.hpp"
#include "../include/DCC.hpp"

#define private public
#include "../include/Server.hpp"
#undef private

namespace {
    void fail(const std::string& message)
    {
        std::cerr << "FAIL partial-send: " << message << std::endl;
        std::exit(1);
    }

    void setNonBlocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
            fail("could not make socket non-blocking");
    }

    void drainSocket(int fd, std::string& output)
    {
        char buffer[16384];
        while (true) {
            ssize_t count = recv(fd, buffer, sizeof(buffer), 0);
            if (count > 0) {
                output.append(buffer, static_cast<size_t>(count));
                continue;
            }
            if (count < 0 && errno == EINTR)
                continue;
            if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return;
            if (count == 0)
                return;
            fail("unexpected receive error");
        }
    }
}

int main()
{
    const std::string expectedDigest =
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad";
    if (SHA256::SHA256Maker("abc") != expectedDigest)
        fail("SHA-256 abc known vector mismatch");
    std::cout << "PASS SHA-256 abc known vector" << std::endl;

    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) < 0)
        fail("socketpair failed");

    int sendBuffer = 1024;
    if (setsockopt(pair[0], SOL_SOCKET, SO_SNDBUF,
                   &sendBuffer, sizeof(sendBuffer)) < 0)
        fail("could not reduce send buffer");
    setNonBlocking(pair[0]);
    setNonBlocking(pair[1]);

    std::string password = "testpass";
    Server server(0, password);
    struct pollfd outputPfd = { pair[0], POLLOUT, 0 };
    server._pfds.push_back(outputPfd);
    const size_t outputIndex = server._pfds.size() - 1;

    std::string expected;
    expected.reserve(1024 * 1024);
    for (size_t i = 0; i < 1024 * 1024; ++i)
        expected += static_cast<char>('A' + (i % 23));

    User user(pair[0]);
    user.addOutbox(expected);
    if (!server._flushOut(user, outputIndex))
        fail("connection closed during initial send");
    if (user.getReferOutbox().empty())
        fail("test did not force a partial send");
    if (user.getOutboxOffset() == 0 || user.getOutboxOffset() >= expected.size())
        fail("partial-send offset was not preserved");

    std::string actual;
    actual.reserve(expected.size());
    size_t rounds = 0;
    while (!user.getReferOutbox().empty() && rounds++ < 10000) {
        drainSocket(pair[1], actual);
        if (!server._flushOut(user, outputIndex))
            fail("connection closed while resuming a partial send");
    }
    drainSocket(pair[1], actual);

    if (!user.getReferOutbox().empty())
        fail("queued message did not finish sending");
    if (user.getOutboxOffset() != 0)
        fail("offset was not reset after the message completed");
    if (actual != expected)
        fail("resumed output was duplicated or truncated");

    close(pair[0]);
    close(pair[1]);

    int brokenPair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, brokenPair) < 0)
        fail("SIGPIPE socketpair failed");
    close(brokenPair[1]);

    User brokenUser(brokenPair[0]);
    brokenUser.addOutbox("peer is already closed");
    struct pollfd brokenPfd = { brokenPair[0], POLLOUT, 0 };
    server._pfds.push_back(brokenPfd);
    if (server._flushOut(brokenUser, server._pfds.size() - 1))
        fail("closed peer was reported as connected");

    std::cout << "PASS partial-send offset, exact stream, SIGPIPE suppression" << std::endl;
    return 0;
}
