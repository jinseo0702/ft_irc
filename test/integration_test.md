# ft_irc 통합 테스트 시나리오

## 1. 서버 시작 테스트
```bash
# 서버 시작
./ft_irc 6667 mypassword

# 예상 결과:
# - 포트 6667에서 리스닝
# - #lobby 채널 자동 생성
# - 봇 자동 초기화
# - "Listening on port 6667, #lobby created" 메시지
```

## 2. 클라이언트 연결 테스트
```bash
# IRC 클라이언트로 연결 (예: irssi, hexchat)
# 또는 nc 사용
nc 127.0.0.1 6667

# 연결 후 명령어 순서:
PASS mypassword
NICK testuser
USER testuser 0 * :Test User
```

## 3. 기본 IRC 명령어 테스트

### 3.1 인증 및 사용자 설정
```
PASS mypassword
NICK testuser
USER testuser 0 * :Test User
```

### 3.2 채널 참여
```
JOIN #test
JOIN #test2 secretkey
```

### 3.3 메시지 전송
```
PRIVMSG #test :Hello everyone!
PRIVMSG testuser2 :Private message
```

### 3.4 채널 운영자 기능
```
MODE #test +o testuser2
KICK #test testuser2 :Reason for kick
INVITE testuser3 #test
TOPIC #test :New channel topic
```

## 4. 봇 기능 테스트

### 4.1 개인 메시지로 봇 명령어
```
PRIVMSG ft_irc_bot :help
PRIVMSG ft_irc_bot :time
PRIVMSG ft_irc_bot :weather 서울
PRIVMSG ft_irc_bot :calc 1 + 2 * 3
PRIVMSG ft_irc_bot :quote
PRIVMSG ft_irc_bot :roll 20
PRIVMSG ft_irc_bot :8ball 내일 비올까?
```

### 4.2 채널에서 봇 명령어
```
PRIVMSG #test :ft_irc_bot: help
PRIVMSG #test :ft_irc_bot: time
PRIVMSG #test :ft_irc_bot: weather 부산
```

## 5. DCC 파일 전송 테스트

### 5.1 파일 전송 요청
```
DCC SEND testuser2 testfile.txt 1024
```

### 5.2 파일 전송 수락/거부
```
DCC ACCEPT testfile.txt 1024
DCC REJECT testfile.txt
```

## 6. 에러 처리 테스트

### 6.1 잘못된 비밀번호
```
PASS wrongpassword
```

### 6.2 잘못된 닉네임
```
NICK invalid@nick
```

### 6.3 존재하지 않는 채널
```
PRIVMSG #nonexistent :Hello
```

### 6.4 권한 없는 명령어
```
MODE #test +o testuser2  # 운영자가 아닌 경우
```

## 7. 부분 데이터 처리 테스트

### 7.1 분할된 명령어 전송
```bash
# nc로 연결 후 명령어를 여러 번에 나누어 전송
echo -n "JOI" | nc 127.0.0.1 6667
echo -n "N #" | nc 127.0.0.1 6667
echo -n "test\r\n" | nc 127.0.0.1 6667
```

## 8. 동시 연결 테스트

### 8.1 여러 클라이언트 동시 연결
```bash
# 터미널 1
nc 127.0.0.1 6667

# 터미널 2
nc 127.0.0.1 6667

# 터미널 3
nc 127.0.0.1 6667
```

### 8.2 채널 브로드캐스트 테스트
```
# 클라이언트 1
JOIN #test
PRIVMSG #test :Hello everyone!

# 클라이언트 2, 3에서 메시지 수신 확인
```

## 9. 서버 종료 테스트

### 9.1 정상 종료
```bash
# 서버 터미널에서 Ctrl+C
# 모든 클라이언트 연결 정리 확인
```

## 10. 예상 결과

### 10.1 성공적인 테스트 결과
- 모든 명령어가 올바르게 처리됨
- 에러 메시지가 적절히 반환됨
- 봇이 명령어에 올바르게 응답함
- DCC 세션이 올바르게 생성/관리됨
- 부분 데이터가 올바르게 처리됨
- 여러 클라이언트가 동시에 연결 가능

### 10.2 확인해야 할 사항
- 메모리 누수 없음
- 파일 디스크립터 누수 없음
- 서버가 예기치 않게 종료되지 않음
- 모든 에러 상황에서 적절한 응답
- C++98 표준 준수 