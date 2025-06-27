#!/bin/bash

echo "=== 간단한 IRC 서버 테스트 ==="
echo

# 1. 빌드 확인
echo "1. 빌드 확인..."
make clean
if make; then
    echo "✅ 빌드 성공"
else
    echo "❌ 빌드 실패"
    exit 1
fi

# 2. 실행 파일 확인
echo "2. 실행 파일 확인..."
if [ -f "./ft_irc" ]; then
    echo "✅ ft_irc 실행 파일 존재"
    ls -la ft_irc
else
    echo "❌ ft_irc 실행 파일 없음"
    exit 1
fi

# 3. 인자 검증
echo "3. 인자 검증..."
echo "인자 없이 실행:"
./ft_irc
echo

echo "잘못된 포트로 실행:"
./ft_irc abc password123
echo

echo "올바른 인자로 실행 (5초간):"
./ft_irc 6667 testpass &
SERVER_PID=$!
sleep 5

# 4. 서버 상태 확인
echo "4. 서버 상태 확인..."
if ps -p $SERVER_PID > /dev/null; then
    echo "✅ 서버 실행 중 (PID: $SERVER_PID)"
    
    # 포트 확인
    if lsof -i :6667 > /dev/null 2>&1; then
        echo "✅ 포트 6667 리스닝 중"
    else
        echo "❌ 포트 6667 리스닝 실패"
    fi
else
    echo "❌ 서버 실행 실패"
fi

# 5. 서버 종료
echo "5. 서버 종료..."
kill $SERVER_PID 2>/dev/null
sleep 1

if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    echo "✅ 서버 정상 종료"
else
    echo "❌ 서버 종료 실패"
    kill -9 $SERVER_PID 2>/dev/null
fi

echo
echo "=== 기본 테스트 완료 ===" 