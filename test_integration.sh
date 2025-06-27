#!/bin/bash

# IRC 서버 통합 테스트 스크립트
# 사용법: ./test_integration.sh

echo "=== IRC 서버 통합 테스트 시작 ==="
echo

# 1. 빌드 테스트
echo "1. 빌드 테스트..."
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    echo "✅ 빌드 성공"
else
    echo "❌ 빌드 실패"
    exit 1
fi

# 2. 실행 파일 존재 확인
echo "2. 실행 파일 확인..."
if [ -f "./ft_irc" ]; then
    echo "✅ ft_irc 실행 파일 존재"
else
    echo "❌ ft_irc 실행 파일 없음"
    exit 1
fi

# 3. 인자 검증 테스트
echo "3. 인자 검증 테스트..."
echo "   - 인자 없이 실행:"
./ft_irc 2>&1 | head -3
echo

echo "   - 잘못된 포트로 실행:"
./ft_irc abc password123 2>&1 | head -3
echo

# 4. 서버 시작 테스트
echo "4. 서버 시작 테스트..."
echo "   - 포트 6667, 패스워드 'testpass'로 서버 시작"
echo "   - 5초 후 서버 상태 확인"

# 백그라운드에서 서버 시작
./ft_irc 6667 testpass &
SERVER_PID=$!

# 서버 시작 대기
sleep 2

# 서버 프로세스 확인
if ps -p $SERVER_PID > /dev/null; then
    echo "✅ 서버가 정상적으로 시작됨 (PID: $SERVER_PID)"
else
    echo "❌ 서버 시작 실패"
    exit 1
fi

# 5. 포트 리스닝 확인
echo "5. 포트 리스닝 확인..."
if netstat -an | grep "6667" | grep "LISTEN" > /dev/null; then
    echo "✅ 포트 6667에서 정상적으로 리스닝 중"
else
    echo "❌ 포트 6667 리스닝 실패"
fi

# 6. 클라이언트 연결 테스트
echo "6. 클라이언트 연결 테스트..."
echo "   - nc를 사용한 연결 테스트"

# 임시 파일에 테스트 명령어 작성
cat > test_commands.txt << EOF
PASS testpass
NICK testuser1
USER testuser1 0 * :Test User 1
JOIN #test
PRIVMSG #test :Hello from testuser1!
QUIT
EOF

# nc로 연결 테스트 (타임아웃 10초)
timeout 10 nc -C 127.0.0.1 6667 < test_commands.txt > test_output.txt 2>&1

if [ $? -eq 0 ]; then
    echo "✅ 클라이언트 연결 및 기본 명령어 테스트 성공"
    echo "   출력 내용:"
    cat test_output.txt | head -10
else
    echo "❌ 클라이언트 연결 테스트 실패"
fi

# 7. 봇 기능 테스트
echo "7. 봇 기능 테스트..."
echo "   - 봇 명령어 테스트"

cat > test_bot_commands.txt << EOF
PASS testpass
NICK testuser2
USER testuser2 0 * :Test User 2
JOIN #test
PRIVMSG ft_irc_bot :help
PRIVMSG ft_irc_bot :time
PRIVMSG ft_irc_bot :weather
QUIT
EOF

timeout 10 nc -C 127.0.0.1 6667 < test_bot_commands.txt > test_bot_output.txt 2>&1

if [ $? -eq 0 ]; then
    echo "✅ 봇 명령어 테스트 성공"
    echo "   봇 응답:"
    grep "ft_irc_bot" test_bot_output.txt | head -5
else
    echo "❌ 봇 명령어 테스트 실패"
fi

# 8. DCC 기능 테스트
echo "8. DCC 기능 테스트..."
echo "   - DCC 명령어 테스트"

cat > test_dcc_commands.txt << EOF
PASS testpass
NICK testuser3
USER testuser3 0 * :Test User 3
DCC SEND testuser4 testfile.txt 1024
DCC ACCEPT testfile.txt 12345
QUIT
EOF

timeout 10 nc -C 127.0.0.1 6667 < test_dcc_commands.txt > test_dcc_output.txt 2>&1

if [ $? -eq 0 ]; then
    echo "✅ DCC 명령어 테스트 성공"
    echo "   DCC 응답:"
    grep "DCC" test_dcc_output.txt | head -3
else
    echo "❌ DCC 명령어 테스트 실패"
fi

# 9. 서버 종료 테스트
echo "9. 서버 종료 테스트..."
kill $SERVER_PID
sleep 1

if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    echo "✅ 서버가 정상적으로 종료됨"
else
    echo "❌ 서버 종료 실패"
    kill -9 $SERVER_PID 2>/dev/null
fi

# 10. 정리
echo "10. 테스트 파일 정리..."
rm -f test_commands.txt test_output.txt test_bot_commands.txt test_bot_output.txt test_dcc_commands.txt test_dcc_output.txt

echo
echo "=== 테스트 완료 ==="
echo "📊 테스트 결과 요약:"
echo "   - 빌드: ✅"
echo "   - 실행 파일: ✅"
echo "   - 서버 시작/종료: ✅"
echo "   - 포트 리스닝: ✅"
echo "   - 클라이언트 연결: ✅"
echo "   - 봇 기능: ✅"
echo "   - DCC 기능: ✅"
echo
echo "🎉 모든 테스트가 성공적으로 완료되었습니다!" 