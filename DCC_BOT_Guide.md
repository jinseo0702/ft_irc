# IRC 서버 DCC & BOT 기능 가이드

## 📋 목차
1. [DCC (Direct Client-to-Client) 파일 전송](#dcc-direct-client-to-client-파일-전송)
2. [IRC 봇 기능](#irc-봇-기능)
3. [사용 예시](#사용-예시)
4. [기술적 구현](#기술적-구현)

---

## 📁 DCC (Direct Client-to-Client) 파일 전송

### 개요
DCC는 IRC 네트워크를 통해 직접적인 파일 전송을 가능하게 하는 프로토콜입니다. 서버를 거치지 않고 클라이언트 간 직접 연결하여 파일을 전송합니다.

### 주요 기능

#### 1. DCC SEND - 파일 전송 요청
```
DCC SEND <수신자> <파일명> <파일크기>
```

**설명:**
- 파일 전송을 요청하는 명령어
- 수신자에게 파일 전송 제안을 보냄
- 파일명과 크기 정보를 포함
- 자동으로 사용 가능한 포트 할당

**예시:**
```
DCC SEND alice document.txt 1024
```

#### 2. DCC ACCEPT - 파일 전송 수락
```
DCC ACCEPT <파일명> <포트번호>
```

**설명:**
- 파일 전송 요청을 수락하는 명령어
- 지정된 포트에서 파일을 수신할 준비
- 파일명과 포트번호를 지정

**예시:**
```
DCC ACCEPT document.txt 12345
```

#### 3. DCC RESUME - 전송 재개
```
DCC RESUME <파일명> <포트번호>
```

**설명:**
- 중단된 파일 전송을 재개하는 명령어
- 이전 전송에서 중단된 지점부터 계속

#### 4. DCC REJECT - 전송 거부
```
DCC REJECT <파일명>
```

**설명:**
- 파일 전송 요청을 거부하는 명령어

### DCC 전송 과정

1. **전송 요청**: 발신자가 `DCC SEND` 명령어로 전송 요청
2. **요청 전달**: 서버가 수신자에게 전송 요청 메시지 전달
3. **수락 처리**: 수신자가 `DCC ACCEPT` 명령어로 수락
4. **직접 연결**: 클라이언트 간 직접 TCP 연결 수립
5. **파일 전송**: 연결된 소켓을 통해 파일 데이터 전송
6. **전송 완료**: 전송 완료 후 연결 종료

### DCC 매니저 기능

- **세션 관리**: 여러 DCC 전송 세션을 동시에 관리
- **포트 할당**: 자동으로 사용 가능한 포트 할당 (1024-65535)
- **전송 상태 추적**: 각 전송의 진행 상황 모니터링
- **에러 처리**: 연결 실패, 전송 중단 등의 예외 상황 처리
- **업로드 디렉토리**: `./uploads` 폴더에 수신 파일 저장
- **IP 주소 관리**: 로컬 IP 주소 자동 감지

---

## 🤖 IRC 봇 기능

### 개요
IRC 봇은 자동화된 응답과 유틸리티 기능을 제공하는 프로그램입니다. 서버에 자동으로 연결되어 사용자들의 요청에 응답합니다.

### 봇 정보
- **봇 이름**: `ft_irc_bot`
- **봇 ID**: 999
- **자동 연결**: 서버 시작 시 자동으로 #lobby 채널에 참가

### 지원 명령어

#### 1. help - 도움말
```
PRIVMSG ft_irc_bot :help
```

**응답:**
```
ft_irc_bot: 사용 가능한 명령어:
  help - 이 도움말을 보여줍니다
  time - 현재 시간을 보여줍니다
  weather [도시] - 날씨 정보를 보여줍니다
  calc [수식] - 수식을 계산합니다
  quote - 랜덤 명언을 보여줍니다
  roll [숫자] - 주사위를 굴립니다
  8ball [질문] - 8번 공에게 질문합니다
```

#### 2. time - 현재 시간
```
PRIVMSG ft_irc_bot :time
```

**응답:**
```
ft_irc_bot: 현재 시간: 2024년 06월 27일 10시 45분 30초
```

#### 3. weather - 날씨 정보
```
PRIVMSG ft_irc_bot :weather
PRIVMSG ft_irc_bot :weather 서울
```

**응답:**
```
ft_irc_bot: 서울 날씨: 맑음, 기온 22°C, 습도 65%
```

#### 4. calc - 수식 계산
```
PRIVMSG ft_irc_bot :calc 2+3*4
PRIVMSG ft_irc_bot :calc 10/2+5
```

**응답:**
```
ft_irc_bot: 2+3*4 = 14
ft_irc_bot: 10/2+5 = 10
```

#### 5. quote - 랜덤 명언
```
PRIVMSG ft_irc_bot :quote
```

**응답:**
```
ft_irc_bot: "삶은 당신이 만나는 사람들로 이루어진다." - 알베르트 아인슈타인
```

#### 6. roll - 주사위 굴리기
```
PRIVMSG ft_irc_bot :roll
PRIVMSG ft_irc_bot :roll 20
```

**응답:**
```
ft_irc_bot: 주사위 결과: 4 (1-6)
ft_irc_bot: 주사위 결과: 15 (1-20)
```

#### 7. 8ball - 8번 공 마법
```
PRIVMSG ft_irc_bot :8ball 내일 비가 올까?
PRIVMSG ft_irc_bot :8ball 이 프로젝트가 성공할까?
```

**응답:**
```
ft_irc_bot: 확실합니다
ft_irc_bot: 그럴 가능성이 높습니다
```

### 봇 응답 형식
- **개인 메시지**: `ft_irc_bot: [응답 내용]`
- **채널 메시지**: `ft_irc_bot: [응답 내용]`
- **에러 메시지**: `ft_irc_bot: 오류 - [에러 내용]`

---

## 💡 사용 예시

### DCC 파일 전송 예시

#### 시나리오: Alice가 Bob에게 파일 전송

1. **Alice의 전송 요청:**
```
PASS password123
NICK alice
USER alice 0 * :Alice User
JOIN #general
DCC SEND bob document.txt 2048
```

2. **Bob이 수신 요청을 받음:**
```
:alice!alice@127.0.0.1 PRIVMSG bob :DCC SEND document.txt 192.168.1.100 54321 2048
```

3. **Bob의 수락:**
```
DCC ACCEPT document.txt 54321
```

4. **파일 전송 시작:**
- Alice와 Bob 간 직접 TCP 연결 수립
- 파일 데이터 전송
- 전송 완료 후 연결 종료

### 봇 사용 예시

#### 기본 대화
```
PASS password123
NICK user1
USER user1 0 * :Test User
JOIN #lobby
PRIVMSG ft_irc_bot :help
PRIVMSG ft_irc_bot :time
PRIVMSG ft_irc_bot :weather 서울
PRIVMSG ft_irc_bot :calc 15+27*3
PRIVMSG ft_irc_bot :roll 12
PRIVMSG ft_irc_bot :8ball 오늘 운세는 어떨까?
```

#### 응답 예시
```
ft_irc_bot: 사용 가능한 명령어:
  help - 이 도움말을 보여줍니다
  time - 현재 시간을 보여줍니다
  weather [도시] - 날씨 정보를 보여줍니다
  calc [수식] - 수식을 계산합니다
  quote - 랜덤 명언을 보여줍니다
  roll [숫자] - 주사위를 굴립니다
  8ball [질문] - 8번 공에게 질문합니다

ft_irc_bot: 현재 시간: 2024년 06월 27일 10시 45분 30초

ft_irc_bot: 서울 날씨: 맑음, 기온 22°C, 습도 65%

ft_irc_bot: 15+27*3 = 96

ft_irc_bot: 주사위 결과: 8 (1-12)

ft_irc_bot: 확실합니다
```

---

## 🔧 기술적 구현

### DCC 구현 구조

#### 클래스 구조
```
DCCSession
├── 파일 전송 세션 관리
├── 소켓 연결 처리
├── 전송 상태 추적
├── 파일 경로 설정
└── 에러 처리

DCCManager
├── 세션 컨테이너 관리
├── 포트 할당 (1024-65535)
├── 전송 스케줄링
├── 업로드 디렉토리 관리
└── 리소스 정리
```

#### 주요 함수
- `DCCSession::setupConnection()`: 연결 설정
- `DCCSession::sendFile()`: 파일 전송
- `DCCSession::receiveFile()`: 파일 수신
- `DCCManager::processDCCTransfers()`: 전송 처리
- `DCCManager::createSession()`: 세션 생성
- `DCCManager::findAvailablePort()`: 사용 가능한 포트 찾기

### 봇 구현 구조

#### 클래스 구조
```
BotCore
├── 봇 초기화
├── 명령어 파싱
├── 응답 생성
├── 메시지 처리
└── 날씨 데이터 관리

BotCommands
├── help 명령어
├── time 명령어
├── weather 명령어
├── calc 명령어 (수식 계산)
├── quote 명령어 (랜덤 명언)
├── roll 명령어 (주사위)
└── 8ball 명령어 (마법 8번 공)
```

#### 주요 함수
- `Bot::executeCommand()`: 명령어 실행
- `Bot::cmdHelp()`: 도움말 처리
- `Bot::cmdTime()`: 시간 처리
- `Bot::cmdWeather()`: 날씨 처리
- `Bot::cmdCalc()`: 수식 계산
- `Bot::cmdQuote()`: 명언 처리
- `Bot::cmdRoll()`: 주사위 처리
- `Bot::cmdEightBall()`: 8번 공 처리
- `Bot::evaluateExpression()`: 수식 평가
- `Bot::getRandomNumber()`: 랜덤 숫자 생성

### 통합 구조

#### Server 클래스 통합
```cpp
class Server {
private:
    DCCManager _dccManager;    // DCC 전송 관리
    Bot _bot;                 // 봇 기능
    
public:
    void run() {
        while (true) {
            // IRC 명령어 처리
            processIRCCommands();
            
            // DCC 전송 처리
            _dccManager.processDCCTransfers();
            
            // 봇 메시지 처리
            _processBotMessages();
        }
    }
};
```

### C++98 표준 준수

- **스마트 포인터**: `SharedPtr` 사용으로 메모리 관리
- **STL 컨테이너**: `std::vector`, `std::map` 사용
- **예외 처리**: `try-catch` 블록으로 안전한 에러 처리
- **RAII**: 생성자/소멸자로 리소스 관리
- **문자열 처리**: `std::string`, `std::ostringstream` 사용
- **시간 처리**: `time()`, `localtime()` 함수 사용

---

## 📝 주의사항

### DCC 사용 시
1. **포트 방화벽**: DCC 전송을 위한 포트가 열려있어야 함
2. **파일 크기**: 큰 파일 전송 시 시간이 오래 걸릴 수 있음
3. **연결 안정성**: 네트워크 불안정 시 전송이 중단될 수 있음
4. **보안**: 신뢰할 수 있는 사용자와만 파일 전송
5. **업로드 디렉토리**: `./uploads` 폴더가 자동으로 생성됨
6. **포트 범위**: 1024-65535 범위에서 자동 포트 할당

### 봇 사용 시
1. **명령어 형식**: 정확한 명령어 형식을 사용해야 함
2. **채널 참가**: 봇이 있는 채널에서만 명령어 사용 가능
3. **응답 지연**: 서버 부하에 따라 응답이 지연될 수 있음
4. **명령어 제한**: 지원하지 않는 명령어는 무시됨
5. **수식 계산**: 기본적인 사칙연산만 지원 (+, -, *, /)
6. **날씨 정보**: 미리 정의된 도시 정보만 제공

---

## 🎯 결론

이 IRC 서버는 DCC 파일 전송과 봇 기능을 완전히 구현하여 현대적인 IRC 서버의 모든 기능을 제공합니다. C++98 표준을 준수하면서도 안정적이고 확장 가능한 구조로 설계되어 있습니다.

### 주요 특징
- **DCC**: 안전하고 효율적인 파일 전송 (SEND, ACCEPT, RESUME, REJECT)
- **봇**: 7가지 유틸리티 명령어 지원 (help, time, weather, calc, quote, roll, 8ball)
- **통합**: 두 기능이 서버에 완벽하게 통합됨
- **확장성**: 새로운 명령어나 기능 추가 용이
- **안정성**: 메모리 누수 방지, 예외 처리, 리소스 관리
- **호환성**: C++98 표준 완전 준수

### 실제 사용 가능한 기능
1. **파일 전송**: 클라이언트 간 직접 파일 전송
2. **시간 확인**: 현재 시간 한국어 표시
3. **날씨 정보**: 도시별 날씨 정보 제공
4. **수식 계산**: 기본 사칙연산 지원
5. **랜덤 기능**: 명언, 주사위, 8번 공 마법
6. **도움말**: 상세한 명령어 가이드 