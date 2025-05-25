# ft_irc
## Internet Relay Chat

**Summary:** This project is about creating your own IRC server.
You will use an actual IRC client to connect to your server and test it.
The Internet is governed by solid standard protocols that allow connected computers to
interact with each other.
It's always beneficial to understand these protocols.

**Version:** 8.2

# ft_irc
## 인터넷 릴레이 채팅

**요약:** 이 프로젝트는 자신만의 IRC 서버를 만드는 것에 관한 것입니다.
실제 IRC 클라이언트를 사용하여 여러분의 서버에 연결하고 테스트할 것입니다.
인터넷은 연결된 컴퓨터들이 서로 상호작용할 수 있게 해주는 견고한 표준 프로토콜에 의해 관리됩니다.
이러한 프로토콜을 이해하는 것은 항상 유익합니다.

**버전:** 8.2

---

# Contents

I [Introduction](#introduction) 2

II [General rules](#general-rules) 3

III [Mandatory Part](#mandatory-part) 4
  III.1 [Requirements](#requirements) 5
  III.2 [For MacOS only](#for-macos-only) 6
  III.3 [Test example](#test-example) 6

IV [Bonus part](#bonus-part) 7

V [Submission and peer-evaluation](#submission-and-peer-evaluation) 8

# 목차

I [소개](#introduction) 2

II [일반 규칙](#general-rules) 3

III [필수 파트](#mandatory-part) 4
  III.1 [요구사항](#requirements) 5
  III.2 [MacOS 전용](#for-macos-only) 6
  III.3 [테스트 예제](#test-example) 6

IV [보너스 파트](#bonus-part) 7

V [제출 및 동료 평가](#submission-and-peer-evaluation) 8

---

# Chapter I
## Introduction

Internet Relay Chat or IRC is a text-based communication protocol on the Internet.
It offers real-time messaging that can be either public or private. Users can exchange
direct messages and join group channels.

IRC clients connect to IRC servers in order to join channels. IRC servers are connected
together to form a network.

# 챕터 I
## 소개

인터넷 릴레이 채팅 또는 IRC는 인터넷 상의 텍스트 기반 통신 프로토콜입니다.
이는 공개 또는 비공개가 가능한 실시간 메시징을 제공합니다. 사용자들은 직접 메시지를 교환하고
그룹 채널에 참여할 수 있습니다.

IRC 클라이언트는 채널에 참여하기 위해 IRC 서버에 연결합니다. IRC 서버들은 
네트워크를 형성하기 위해 서로 연결됩니다.

---

# Chapter II
## General rules

* Your program should not crash in any circumstances (even when it runs out of
memory), and should not quit unexpectedly.
If it happens, your project will be considered non-functional and your grade will be
0.

* You have to turn in a Makefile which will compile your source files. It must not
perform unnecessary relinking.

* Your Makefile must at least contain the rules:
$(NAME), all, clean, fclean and re.

* Compile your code with c++ using the flags -Wall -Wextra -Werror.

* Your code must comply with the C++ 98 standard. Then, it should still compile
if you add the flag -std=c++98.

* Try to always code using C++ features when available (for example, choose <cstring>
over <string.h>). You are allowed to use C functions, but always prefer their C++
versions if possible.

* Any external library and Boost libraries are forbidden.

# 챕터 II
## 일반 규칙

* 프로그램은 어떤 상황에서도 충돌해서는 안 되며(메모리 부족 시에도), 
예기치 않게 종료되어서는 안 됩니다.
만약 이런 일이 발생하면, 프로젝트는 기능이 없는 것으로 간주되고 점수는 
0점이 될 것입니다.

* 소스 파일을 컴파일할 Makefile을 제출해야 합니다. 이는 불필요한 
재링크를 수행해서는 안 됩니다.

* Makefile은 최소한 다음 규칙들을 포함해야 합니다:
$(NAME), all, clean, fclean 및 re.

* c++를 사용하여 코드를 컴파일하고 플래그 -Wall -Wextra -Werror를 사용하십시오.

* 코드는 C++ 98 표준을 준수해야 합니다. 그러면 플래그 -std=c++98을 
추가해도 컴파일이 계속 되어야 합니다.

* 가능할 때마다 항상 C++ 기능을 사용하여 코딩하도록 노력하세요(예를 들어, <string.h> 대신 
<cstring>을 선택하세요). C 함수를 사용할 수 있지만, 가능하면 항상 C++ 버전을 
선호하세요.

* 모든 외부 라이브러리 및 Boost 라이브러리는 금지됩니다.

---

# Chapter III
## Mandatory Part

| Program name | ircserv |
|--------------|---------|
| Turn in files | Makefile, *.{h, hpp}, *.cpp, *.tpp, *.ipp, an optional configuration file |
| Makefile | NAME, all, clean, fclean, re |
| Arguments | port: The listening port password: The connection password |
| External functs. | Everything in C++ 98. socket, close, setsockopt, getsockname, getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo, bind, connect, listen, accept, htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa, send, recv, signal, sigaction, lseek, fstat, fcntl, poll (or equivalent) |
| Libft authorized | n/a |
| Description | An IRC server in C++ 98 |

You are required to develop an IRC server using the C++ 98 standard.

You must not develop an IRC client.
You must not implement server-to-server communication.

Your executable will be run as follows:
./ircserv <port> <password>

* port: The port number on which your IRC server will be listening for incoming
IRC connections.

* password: The connection password. It will be needed by any IRC client that tries
to connect to your server.

Even though poll() is mentioned in the subject and the evaluation
scale, you may use any equivalent such as select(), kqueue(), or
epoll().

# 챕터 III
## 필수 파트

| 프로그램 이름 | ircserv |
|--------------|---------|
| 제출 파일 | Makefile, *.{h, hpp}, *.cpp, *.tpp, *.ipp, 선택적 설정 파일 |
| Makefile | NAME, all, clean, fclean, re |
| 인자 | port: 리스닝 포트 password: 연결 비밀번호 |
| 외부 함수 | C++ 98의 모든 것. socket, close, setsockopt, getsockname, getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo, bind, connect, listen, accept, htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa, send, recv, signal, sigaction, lseek, fstat, fcntl, poll (또는 동등한 것) |
| Libft 허용 | 해당 없음 |
| 설명 | C++ 98로 작성된 IRC 서버 |

C++ 98 표준을 사용하여 IRC 서버를 개발해야 합니다.

IRC 클라이언트를 개발해서는 안 됩니다.
서버 간 통신을 구현해서는 안 됩니다.

실행 파일은 다음과 같이 실행될 것입니다:
./ircserv <port> <password>

* port: IRC 서버가 들어오는 IRC 연결을 위해 리스닝할 포트 번호입니다.

* password: 연결 비밀번호입니다. 서버에 연결하려는 모든 IRC 클라이언트에게 필요할 것입니다.

poll()이 주제와 평가 척도에 언급되어 있지만, select(), kqueue(), 또는 
epoll()과 같은 동등한 것을 사용할 수 있습니다.

---

# ft_irc
## Internet Relay Chat

### III.1 Requirements

* The server must be capable of handling multiple clients simultaneously without
hanging.

* Forking is prohibited. All I/O operations must be non-blocking.

* Only 1 poll() (or equivalent) can be used for handling all these operations (read,
write, but also listen, and so forth).

Because you have to use non-blocking file descriptors, it is
possible to use read/recv or write/send functions with no poll()
(or equivalent), and your server wouldn't be blocking.
However, it would consume more system resources.
Therefore, if you attempt to read/recv or write/send in any file
descriptor without using poll() (or equivalent), your grade will be
0.

* Several IRC clients exist. You have to choose one of them as a reference. Your
reference client will be used during the evaluation process.

* Your reference client must be able to connect to your server without encountering
any error.

* Communication between client and server has to be done via TCP/IP (v4 or v6).

* Using your reference client with your server must be similar to using it with any
official IRC server. However, you only have to implement the following features:

  o You must be able to authenticate, set a nickname, a username, join a channel,
  send and receive private messages using your reference client.
  
  o All the messages sent from one client to a channel have to be forwarded to
  every other client that joined the channel.
  
  o You must have operators and regular users.
  
  o Then, you have to implement the commands that are specific to channel
  operators:
  
    * KICK - Eject a client from the channel
    
    * INVITE - Invite a client to a channel
    
    * TOPIC - Change or view the channel topic
    
    * MODE - Change the channel's mode:
    
      · i: Set/remove Invite-only channel
      
      · t: Set/remove the restrictions of the TOPIC command to channel
      operators
      
      · k: Set/remove the channel key (password)

# ft_irc
## 인터넷 릴레이 채팅

### III.1 요구사항

* 서버는 정지 없이 여러 클라이언트를 동시에 처리할 수 있어야 합니다.

* 포크는 금지됩니다. 모든 I/O 작업은 논블로킹이어야 합니다.

* 모든 작업(읽기, 쓰기, 리스닝 등)을 처리하기 위해 단 1개의 poll()(또는 동등한 것)만 사용할 수 있습니다.

논블로킹 파일 디스크립터를 사용해야 하기 때문에, poll()(또는 동등한 것) 
없이 read/recv 또는 write/send 함수를 사용할 수 있으며, 서버가 블로킹되지 않을 것입니다.
그러나, 이는 더 많은 시스템 리소스를 소비할 것입니다.
따라서, poll()(또는 동등한 것)을 사용하지 않고 어떤 파일 디스크립터에서도 
read/recv 또는 write/send를 시도한다면, 점수는 0점이 될 것입니다.

* 여러 IRC 클라이언트가 존재합니다. 그 중 하나를 참조로 선택해야 합니다. 참조 
클라이언트는 평가 과정에서 사용될 것입니다.

* 참조 클라이언트는 오류 없이 서버에 연결할 수 있어야 합니다.

* 클라이언트와 서버 간의 통신은 TCP/IP(v4 또는 v6)를 통해 이루어져야 합니다.

* 참조 클라이언트로 서버를 사용하는 것은 공식 IRC 서버를 사용하는 것과 
유사해야 합니다. 그러나, 다음 기능만 구현하면 됩니다:

  o 참조 클라이언트를 사용하여 인증, 닉네임 설정, 사용자 이름 설정, 채널 참여, 
  개인 메시지 보내기 및 받기가 가능해야 합니다.
  
  o 한 클라이언트가 채널로 보낸 모든 메시지는 해당 채널에 참여한 
  다른 모든 클라이언트에게 전달되어야 합니다.
  
  o 운영자와 일반 사용자가 있어야 합니다.
  
  o 그런 다음, 채널 운영자에게 특정한 명령을 구현해야 합니다:
  
    * KICK - 채널에서 클라이언트 퇴출
    
    * INVITE - 채널에 클라이언트 초대
    
    * TOPIC - 채널 주제 변경 또는 보기
    
    * MODE - 채널의 모드 변경:
    
      · i: 초대 전용 채널 설정/제거
      
      · t: 채널 운영자에 대한 TOPIC 명령의 제한 설정/제거
      
      · k: 채널 키(비밀번호) 설정/제거
      
      · o: Give/take channel operator privilege  
      
      · l: Set/remove the user limit to channel  

* Of course, you are expected to write a clean code.  

---

# ft_irc
## Internet Relay Chat


### III.2 For MacOS only

Since MacOS does not implement write() in the same way as other Unix
OSes, you are permitted to use fcntl().
You must use file descriptors in non-blocking mode in order to get a
behavior similar to the one of other Unix OSes.

However, you are allowed to use fcntl() only as follows:
fcntl(fd, F_SETFL, O_NONBLOCK);
Any other flag is forbidden.

### III.3 Test example

Verify every possible error and issue, such as receiving partial data, low bandwidth, etc.

To ensure that your server correctly processes all data sent to it, the following simple
test using nc can be performed:
```
$> nc -C 127.0.0.1 6667
com^Dman^Dd
$>
```

Use ctrl+D to send the command in several parts: 'com', then 'man', then 'd\n'.

In order to process a command, you have to first aggregate the received packets in
order to rebuild it.

# ft_irc
## 인터넷 릴레이 채팅

      · o: 채널 운영자 권한 부여/회수
      · l: 채널에 대한 사용자 제한 설정/제거

* 물론, 깨끗한 코드를 작성해야 합니다.

### III.2 MacOS 전용

MacOS는 다른 Unix OS와 같은 방식으로 write()를 구현하지 않기 때문에, 
fcntl()을 사용할 수 있습니다.
다른 Unix OS와 유사한 동작을 얻기 위해 논블로킹 모드에서 
파일 디스크립터를 사용해야 합니다.

그러나, 다음과 같이 fcntl()만 사용할 수 있습니다:
fcntl(fd, F_SETFL, O_NONBLOCK);
다른 플래그는 금지됩니다.

### III.3 테스트 예제

부분 데이터 수신, 낮은 대역폭 등과 같은 모든 가능한 오류와 문제를 확인하세요.

서버가 보내진 모든 데이터를 올바르게 처리하는지 확인하기 위해, 다음과 같은 간단한 
nc를 사용한 테스트를 수행할 수 있습니다:
```
$> nc -C 127.0.0.1 6667
com^Dman^Dd
$>
```

ctrl+D를 사용하여 명령을 여러 부분으로 보냅니다: 'com', 그 다음 'man', 그 다음 'd\n'.

명령을 처리하기 위해, 먼저 수신된 패킷을 집계하여 
재구성해야 합니다.

---

# Chapter IV
## Bonus part

Here are additional features you may add to your IRC server to make it resemble an
actual IRC server more closely:

* Handle file transfer.

* A bot.

The bonus part will only be assessed if the mandatory part is
PERFECT. Perfect means the mandatory part has been integrally done
and works without malfunctioning. If you have not passed ALL the
mandatory requirements, your bonus part will not be evaluated at all.

# 챕터 IV
## 보너스 파트

다음은 IRC 서버를 실제 IRC 서버와 더 유사하게 만들기 위해 추가할 수 있는 
기능들입니다:

* 파일 전송 처리.

* 봇.

보너스 파트는 필수 파트가 완벽한 경우에만 평가됩니다. 
완벽함이란 필수 파트가 온전히 완료되고 오작동 없이 작동함을 의미합니다. 
모든 필수 요구사항을 통과하지 못했다면, 보너스 파트는 전혀 평가되지 않을 것입니다.

---

# Chapter V
## Submission and peer-evaluation

Submit your assignment to your Git repository as usual. Only the work inside your
repository will be evaluated during the defense. Do not hesitate to double-check the
names of your files to ensure they are correct.

You are encouraged to create test programs for your project even though they will
not be submitted or graded. Those tests could be especially useful to test your server
during defense, but also your peer's if you have to evaluate another ft_irc one day.
Indeed, you are free to use whatever tests you need during the evaluation process.

Your reference client will be used during the evaluation process.

16D85ACC441674FBA2DF65190663F432222F81AA0248081A7C1C1823F7A96F0B74495
15056E97427E5B22F07132659EC8D88B574BD62C94BB654D5835AAD889B014E078705
709F6E02

# 챕터 V
## 제출 및 동료 평가

평소와 같이 Git 저장소에 과제를 제출하세요. 방어 중에는 저장소 내의 작업만 
평가됩니다. 파일 이름이 올바른지 확인하기 위해 꼭 다시 확인하세요.

제출되거나 채점되지 않더라도 프로젝트를 위한 테스트 프로그램을 만드는 것이 
좋습니다. 이러한 테스트는 방어 중에 서버를 테스트하는 데 특히 유용할 수 있으며, 
언젠가 다른 ft_irc를 평가해야 할 경우 동료의 것을 테스트하는 데도 유용할 수 있습니다.
실제로, 평가 과정에서 필요한 어떤 테스트도 자유롭게 사용할 수 있습니다.

평가 과정에서 참조 클라이언트가 사용될 것입니다.

16D85ACC441674FBA2DF65190663F432222F81AA0248081A7C1C1823F7A96F0B74495
15056E97427E5B22F07132659EC8D88B574BD62C94BB654D5835AAD889B014E078705
709F6E02