# ft_irc

C++98과 `poll()`로 구현한 다중 사용자 IRC 서버입니다. TCP stream을 IRC line 단위로 조립하고, 사용자 등록부터 채널 입장·메시지 전달·권한 변경까지 하나의 이벤트 루프에서 처리합니다.

## 프로젝트 개요

| 항목 | 내용 |
| --- | --- |
| 개발 기간 | 2025.05-2025.11 (핵심 기능 개발: 2025.05-06) |
| 인원 | 2명 |
| 환경 | C++98, Linux, TCP/IP, `poll`, Make |
| 실행 파일 | `ircserv` |
| 핵심 범위 | 다중 접속, 사용자 등록, 채널 상태, IRC 명령, numeric reply |

## 실행 구조

```text
TCP client
   |
   v
accept -- poll(POLLIN) -- User input buffer -- CRLF line split
                                                    |
                                                    v
                             Parser -- command dispatch -- state update
                                                                    |
                                                                    v
TCP client <-- send(POLLOUT) <-- User outbox <-- IRC reply/broadcast
```

- `Server`는 listen socket, `pollfd` 목록, 전체 User와 Channel을 관리합니다.
- `User`는 연결 fd, 등록 상태, 수신 버퍼, 송신 큐를 가집니다.
- `Channel`은 topic, mode, 초대 목록, 참여자를 관리합니다.
- `ChannelData`는 채널 안에서의 User 참조와 operator 권한을 저장합니다.
- `Parser`는 prefix, command, parameter를 분리하고 IRC 문법 범위를 검사합니다.

TCP는 메시지 경계를 보장하지 않으므로 `recv()` 결과를 바로 명령 하나로 처리하지 않습니다. User별 input buffer에 데이터를 누적하고 newline 경계가 완성된 line만 Parser에 넘기며, CRLF 입력에서는 마지막 `\r`을 제거합니다. Listen socket과 accept된 client fd를 모두 non-blocking으로 설정하고, IRC wire line은 CRLF를 포함해 512 bytes로 제한합니다. 부분 `send()`가 발생하면 User별 offset을 보존해 다음 `POLLOUT`에서 이어 보냅니다.

## 빌드와 실행

Linux와 `clang++` 또는 `g++`가 필요합니다.

```bash
make
./ircserv <port> <password>
```

예시:

```bash
./ircserv 6667 secret
```

별도 터미널에서 `nc`로 접속할 수 있습니다.

```bash
nc 127.0.0.1 6667
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #game
PRIVMSG #game :hello
```

종료 및 재빌드:

```bash
make fclean
make re
```

## 구현한 IRC 명령

| 분류 | 명령 | 내용 |
| --- | --- | --- |
| 등록 | `PASS`, `NICK`, `USER`, `QUIT` | 서버 비밀번호 확인, 사용자 등록, 연결 종료 |
| 채널 | `JOIN`, `PART`, `KICK`, `INVITE` | 채널 생성·입장·퇴장, 추방, 초대 |
| 메시지 | `PRIVMSG`, `NOTICE` | 채널 broadcast와 사용자 간 메시지 |
| 채널 상태 | `TOPIC`, `MODE` | topic과 채널 mode 변경 |
| 확인 | `LIST`, `SHOW` | 채널과 사용자 상태 확인용 명령 |

지원하는 Channel mode:

| Mode | 동작 |
| --- | --- |
| `+i` / `-i` | invite-only 설정·해제 |
| `+t` / `-t` | operator만 topic을 변경하도록 설정·해제 |
| `+k` / `-k` | 채널 key 설정·해제 |
| `+o` / `-o` | operator 권한 부여·회수 |
| `+l` / `-l` | 채널 인원 제한 설정·해제 |

입력 계층은 CRLF를 포함한 512-byte wire line 제한을 적용합니다. Parser는 command를 대문자로 정규화하고, 최대 15개 parameter 범위에서 prefix·command·parameter 형식을 확인합니다. 개행 없이 제한을 채운 연결은 버퍼가 계속 커지기 전에 종료합니다.

## 사용자 등록 흐름

```text
CONNECTED
   +-- PASS 성공
        +-- NICK + USER 등록
             +-- ACTIVE
                  +-- JOIN / PRIVMSG / MODE ...
```

등록 전에는 허용된 명령만 처리합니다. `PASS`가 성공한 뒤 `NICK`과 `USER`가 모두 설정되면 일반 채널 명령을 사용할 수 있습니다. 서버 password는 평문 대신 SHA-256 digest로 저장해 입력값의 digest와 비교합니다.

## 주요 설계 판단

### 1. User와 Channel의 소유권 분리

C++98에서는 `std::shared_ptr`를 사용할 수 없어 프로젝트 범위의 reference-counted [`SharedPtr<T>`](https://github.com/jinseo0702/ft_irc/blob/main/include/SharedPtr.hpp)를 직접 설계·구현했습니다. Server와 여러 Channel이 같은 User를 참조해도 한쪽 컨테이너의 변화 때문에 객체가 먼저 파괴되지 않도록 했습니다.

이 구현은 단일 스레드 사용을 전제로 하며 weak reference와 cycle 처리는 지원하지 않습니다.

### 2. Server ID와 Channel membership ID를 구분

전체 사용자 저장소의 ID와 각 Channel 내부 membership ID는 서로 다른 순서로 생성됩니다. 두 값을 같은 ID로 취급하면 다른 사용자를 찾을 수 있어, Server ID로 Channel membership을 검색하는 변환 함수를 두었습니다.

### 3. index 대신 fd로 이벤트의 사용자를 찾기

초기 구현은 `pollfd`의 index와 User 저장소 index가 같다고 가정했습니다. 사용자가 종료되면 `pollfd`는 erase되어 순서가 바뀌지만 User ID는 유지되기 때문에, 재접속 과정에서 잘못된 User를 찾고 segmentation fault가 발생했습니다.

수정 후에는 이벤트가 발생한 `pollfd.fd`와 같은 fd를 가진 User를 조회합니다. 연결 종료 시에는 socket과 poll entry뿐 아니라 Channel membership과 invite도 함께 정리한 뒤 User를 inactive 상태로 표시합니다. 분석 과정은 [Doc/ModifyDoc.MD](https://github.com/jinseo0702/ft_irc/blob/main/Doc/ModifyDoc.MD)에 정리했습니다.

## 개인 기여

- C++98 reference-counted `SharedPtr<T>` 직접 설계·구현
- IRC line parser와 문법 검사: `Parser`, `Utils`
- command·error code mapping: `Rulehandle`, `Rule`
- SHA-256 기반 password 비교와 `PASS` 등록 단계
- Server ID와 Channel membership ID 변환 로직
- JOIN·PRIVMSG 통합 과정의 오류 수정
- disconnect crash, event loop 무한 반복, fd/index 불일치 분석
- LLDB, Valgrind, Callgrind 로그와 수정 기록 정리

개인 브랜치에서 구현한 코드를 통합 브랜치에서 팀원의 Server·Channel 코드와 합쳤습니다. 통합 과정에서 발견한 문제는 재현 조건과 관련 함수를 작업 로그에 남겼습니다.

## 저장소 구조

```text
.
+-- include/
|   +-- Server.hpp          # event loop와 command handler
|   +-- User.hpp            # 연결·등록 상태·buffer/outbox
|   +-- Channel.hpp         # 채널 상태와 membership
|   +-- Parser.hpp          # IRC line parser
|   +-- SharedPtr.hpp       # C++98 reference-counted pointer
|   +-- TotalDatabase.hpp   # ID 기반 객체 저장소
|   +-- Password.hpp        # password digest 비교
+-- srcs/
|   +-- Server.cpp          # socket I/O와 IRC 명령 처리
|   +-- Parser.cpp          # line parsing과 validation
|   +-- Channel.cpp         # channel state와 broadcast
|   +-- User.cpp            # user state와 numeric reply
|   +-- ...
+-- Doc/
|   +-- IRC_ABNF.MD         # parser 기준
|   +-- ModifyDoc.MD        # 문제와 수정 기록
|   +-- analyze/            # LLDB·Valgrind·Callgrind 로그
+-- Makefile
```

## 재현 가능한 검증

2026년 9월 기준 `clang++ -Wall -Wextra -Werror -std=c++98` 빌드와 아래 자동 회귀검사를 확인했습니다.

```bash
make test
```

| Suite | 결과 | 확인 범위 |
| --- | --- | --- |
| `tests/outbox_regression.cpp` | 1/1 PASS | SHA-256 `abc` known vector, 강제 partial send, 1 MiB byte stream 일치, offset 복구, SIGPIPE 억제 |
| `tests/regression.py` | 10/10 PASS | accepted fd non-blocking, TCP 경계, 512-byte 제한, NOTICE 안전성, HUP 정리·nickname 재사용, invite-only, 빈 USER 입력 검증, 채널 인원 제한, 입력 buffer 제한, password log 비노출 |

검사는 socket flag, 서버 생존 여부, IRC reply, 최종 수신 byte stream을 assertion으로 판정합니다.

### 생성형 AI 활용 범위

2026년 보완 과정에서 생성형 AI를 문제 가설, patch 후보, 경계값 test 설계에 사용했습니다. 제안은 기존 설계와 비교해 선택했고, 최종 판단은 코드 review와 `make test`의 build·실행 결과로 확인했습니다.

핵심 IRC 흐름과 별도로 Bot, DCC file transfer 실험 코드가 포함되어 있습니다. 이 확장 기능은 전체 시나리오를 검증하지 않았습니다.

## 참고 문서

- [IRC message ABNF 정리](https://github.com/jinseo0702/ft_irc/blob/main/Doc/IRC_ABNF.MD)
- [구현·디버깅 작업 로그](https://github.com/jinseo0702/ft_irc/blob/main/Doc/ModifyDoc.MD)
- [GitHub repository](https://github.com/jinseo0702/ft_irc)
