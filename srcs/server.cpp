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

bool Server::readflag = 0;

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
    this->_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_listenFd < 0)
    {
        perror("socket");
        std::exit(1);
    }
    int yes = 1;
    setsockopt(this->_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in a; std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(port);

    if (bind(this->_listenFd, (struct sockaddr*)&a, sizeof(a)) < 0)
    {
        perror("bind");
        std::exit(1);
    }
    if (listen(this->_listenFd, 20) < 0)
    {
        perror("listen");
        std::exit(1);
    }
    fcntl(this->_listenFd, F_SETFL, O_NONBLOCK);
    struct pollfd pfd;
    pfd.fd = this->_listenFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_pfds.push_back(pfd);
}

/*
지금 문제가 발생한 부분은 
while (i < this->_pfds.size())
이 루프 안에서 사용자는 _user2의 데이터는 0부터 시작하는데 _pfds는 fd 3(index = 0)을 제외한 index =1 부터
시작해서 없는 곳을 계속해서 바라 보는 문제가 발생했습니다. 그래서 for문을 돌때 항상 i - 1 = index로 규정을해서
둘의 엇갈림을 제어 해줬습니다.
이 문제를 해결하고 나니 또문제가 발생을 하게 되었는데, 이번에 발생한 문제는 채팅이 무한 루프가 돌면서 계속해서
기본 입장 축하문이 반복되는 현상이 나타났습니다.(:server NOTICE * :Welcome to #lobby)
이 문제는 살펴보니 _flushOut의 조건에서 원본 u.getOutbox().pop(); 에서 문제가 발생 했습니다.
문제가 발생한 원인은 이 함수는 멤버큐의 임시객체를 반환하게 되었는데 이 que는 임시 복사객체이기 때문에 원본 멤버Que의 값을 변경할 수 없습니다.
그래서 보안문제가 발생 할 수 도 있지만
getReferOutbox() 라는 참조자를 반환하는 함수를 만들어서 pop을 할 때 실제 문자를 지울 수 있도록 만들었습니다.
같은 이유로 getReferIbuf() 함수를 만들어서 erase를 수원하게 할 수 있도록 만들었습니다.
이 Refer가 붙은 get은 원본데이터를 건들여서 정보가 훼소될 가능성이 높기때문에 사용시 주의를 기울여야합니다.
이 문제를 해결하고 나니 또 다른 문제가 발생했습니다. user가 3명이라고 할때 마지막에 입장한 유저가 글을 입력하면 첫번 째 두번째 유저는 출력이 씹히다가 글을 입력할때 나머지 버퍼가 출력되는 문제가 발생했습니다.
지금 문제를 Ai 그리고 내가 예상을 해본 결과 
~~~
            if (this->readflag == true)
                this->_pfds[i].events |= POLLOUT;
~~~
이부분에서 순차적으로 적용이되서 문제가 발생한것 같은 느낌이 듭니다.
이걸 보시고 제가 고치지 않았다면 고쳐주세요!!! 이야호
*/

/* ───────── 메인 루프 ────── */
void Server::run()
{
    while (true)
    {
        if (poll(&this->_pfds[0], this->_pfds.size(), -1) < 0){
            perror("poll");
            break;
        }

        /* (A) 새 연결인지? */
        if (this->_pfds[0].revents & POLLIN)
            _acceptClient();

        /* (B) 손님별 I/O 처리하기 */
        size_t i = 1;
        while (i < this->_pfds.size())
        {
			int index = i - 1; // _pfds[0]는 listen socket이므로, 실제 사용자 소켓은 1부터 시작합니다.
            int fd = this->_pfds[i].fd;//여기서는 i가 1부터 시작하는게 맞습니다.
            TotalDatabase<User>::it it = this->_users2.getUserData(index);//여기서는 i가 1부터 시작하면 안됩니다. 0부터 시작해야합니다. 유저는의 key는 0부터 시작하기 때문입니다.
            // if (it != this->_users2.end()){ 
            if (this->_pfds[i].revents & POLLIN)
                _readLines(*it->second, i);
            // }
            if (this->readflag == true)
                this->_pfds[i].events |= POLLOUT;
            // if (it != this->_users2.end()){
            if (i < this->_pfds.size() && (this->_pfds[i].revents & POLLOUT))
                _flushOut (*it->second, i);
            // }
            ++i;
        }
        this->readflag = false;
        std::cerr << "infinity run :" <<std::endl;
    }
}

/*
this->_users2.returnSecond(id)->setUserName(name);
이 함수가
TotalDatabase<User>::it it = this->_users2.getUserData(id);
it->second->setUserName(name);
와 같은 의미입니다.
이게 더 짧고 가독성이 좋습니다.
근데 이해하기 어렵기 때문에 그냥
SharedPtr<User> findUser = this->_users2.returnSecond(id);
이렇게 한다음
if(findUser.is_valid() == true){
	findUser->setUserName(name);
}
이렇게 사용하는게 가독성도 좋고 더 나은 방법 같습니다. it을 사용 하는 방법도 나쁘지는 않지만 it은 first와 second를 사용해야 하기 때문에 가독성이 떨어집니다.
저 위의 방식을 사용하게 된다면 순환 참조를 조심해야 합니다.
*/

/*
지금문제가 발생하는지점은 // this->_users.insert(std::make_pair(cfd, User(cfd)));//원본
이부분에서는 cfd를 키로 사용해서 User객체를 생성하고 있습니다.
그리고 _users2.addUserWithId(new User(cfd)); 이부분에서는 User객체를 생성하고 있습니다.
이렇게 되면 _users2에 추가된 User객체는 cfd를 키로 사용하지 않고, id를 키로 사용합니다.

*/

/* ───────── accept ──────── */
void Server::_acceptClient()
{
    int cfd = accept(this->_listenFd, 0, 0);
    if (cfd < 0)
        return;
    fcntl(cfd, F_SETFL, O_NONBLOCK);

    static int id = 0;
    struct pollfd pfd;
    pfd.fd = cfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_pfds.push_back(pfd);

    
    // this->_users.insert(std::make_pair(cfd, User(cfd)));//원본
    this->_users2.addUserWithId(new User(cfd));

    TotalDatabase<User>::it it = this->_users2.getUserData(id);
    std::string name = "Local" + std::to_string(id);
    it->second->setUserName(name);
    std::cerr << it->second->getUserName() << std::endl;
    std::string nickname = "NickNmae" + std::to_string(id);
    it->second->setNickName(nickname);
    // this->_lobby.users.insert(&this->_users[cfd]);//원본
    this->_lobby.addUser(it->second);

    std::cout << " + client fd=" << cfd << " joined #lobby" << std::endl;
    it->second->addOutbox(":server NOTICE * :Welcome to #lobby\r\n");
    this->_pfds.back().events |= POLLOUT; // 바로 송신 시도
    id++;
}

/* ───────── 읽기 + 줄 분할 ─  나중에 여기서 파싱한 명령어를 처리하도록 만들기 */
void Server::_readLines(User& u, size_t idx)
{
    char buf[512];
    // ssize_t n = recv(u.fd, buf, sizeof(buf)-1, 0);//원본
    ssize_t n = recv(u.getFd(), buf, sizeof(buf)-1, 0);
    if (n <= 0) // EOF or error → 정리
    {
        this->_pfds.back().events |= POLLOUT; // 바로 송신 시도
        std::cout << " - client fd=" << u.getFd() << " quit" << std::endl;
        close(u.getFd());
        // this->_lobby.users.erase(&u);//원본
        // this->_users.erase(u.fd);// 원본
        this->_pfds.erase(this->_pfds.begin() + idx);//원본
        return;
    }
    buf[n] = '\0';
    // u.ibuf.append(buf, n);//원본
    u.getReferIbuf().append(buf, n);

    size_t pos;
    //IRC는 모든 client의 정보를 추적해야 한다고 프로토콜에 나와 있어서 관련 정보를 추가 하겠습니다.
    std::cout << "users fd :" << u.getFd() <<std::endl;
    std::cout << "users input :" << u.getIbuf() <<std::endl;
    // while ((pos = u.ibuf.find('\n')) != std::string::npos)//원본
    std::cerr << "Message ASCII values: ";
    for (size_t i = 0; buf[i] != '\0'; ++i) {
        std::cerr << (int)buf[i] << " ";
        std::cerr <<  buf[i] << " ";
    }
    while ((pos = u.getReferIbuf().find('\n')) != std::string::npos)
    {
        std::cerr << "infinity readlien :" << std::endl;
        std::string line = u.getReferIbuf().substr(0, pos);
        if (!line.empty() && line[line.size()-1] == '\r')
            line.erase(line.size()-1, 1);
        u.getReferIbuf().erase(0, pos + 1);

        /* “명령어” 따로 없음 – 파싱 이후에 집어 넣을 것 */
        std::string msg = ":" + _fdToStr(u.getFd())
                        + " PRIVMSG #lobby :" + line + "\r\n";
        this->_lobby.broadcast(msg, &u);
    }
    for(int i = idx - 1; i < this->_pfds.size(); i++){
        if (this->_pfds[i].fd == u.getFd())
            this->_pfds[i].events |= POLLOUT; // 송신 대기
    }
    this->readflag = true;
}

/* ───────── 송신 버퍼 비우기 ─ */
void Server::_flushOut(User& u, size_t idx)
{
    // while (!u.outbox.empty())
    while (!u.getOutbox().empty())
    {
        // std::cerr << "infinity flushOut :" << std::endl;
        // const std::string& m = u.outbox.front();//원본
        const std::string m = u.getOutbox().front();
        ssize_t n = send(u.getFd(), m.c_str(), m.size(), 0);
        if (n == (ssize_t)m.size())
            u.getReferOutbox().pop();
        else
            break;               // 다 못 보냈으면 다음 POLLOUT 때 재도전하기
    }
    if (u.getOutbox().empty())
        this->_pfds[idx].events &= ~POLLOUT;  // 대기 해제e
}

/* ────── fd → string  ────── */
std::string Server::_fdToStr(int fd) const
{
    std::ostringstream oss;
    oss << fd;
    return oss.str();
}
