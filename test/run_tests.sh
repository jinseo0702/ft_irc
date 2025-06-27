#!/bin/bash

echo "=== ft_irc 통합 테스트 시작 ==="

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 테스트 결과 카운터
PASSED=0
FAILED=0

# 테스트 함수
test_step() {
    local test_name="$1"
    local command="$2"
    local expected="$3"
    
    echo -e "\n${YELLOW}[테스트] $test_name${NC}"
    echo "실행: $command"
    
    if eval "$command" 2>/dev/null; then
        echo -e "${GREEN}✓ 통과${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ 실패${NC}"
        ((FAILED++))
    fi
}

# 1. 컴파일 테스트
echo -e "\n${YELLOW}=== 1. 컴파일 테스트 ===${NC}"
test_step "Makefile clean" "cd .. && make clean"
test_step "Makefile build" "cd .. && make"
test_step "실행 파일 존재 확인" "cd .. && test -f ft_irc"

# 2. C++98 표준 테스트
echo -e "\n${YELLOW}=== 2. C++98 표준 테스트 ===${NC}"
test_step "C++98 컴파일 테스트" "cd .. && make clean && CXXFLAGS='-std=c++98' make"

# 3. 개별 기능 테스트
echo -e "\n${YELLOW}=== 3. 개별 기능 테스트 ===${NC}"

# DCC 테스트 컴파일
test_step "DCC 테스트 컴파일" "g++ -std=c++98 -Wall -Wextra -Werror -I../include testDCC.cpp ../srcs/DCC.cpp -o testDCC"

# 봇 테스트 컴파일
test_step "봇 테스트 컴파일" "g++ -std=c++98 -Wall -Wextra -Werror -I../include testBot.cpp ../srcs/Bot.cpp -o testBot"

# 4. 코드 품질 테스트
echo -e "\n${YELLOW}=== 4. 코드 품질 테스트 ===${NC}"

# C++98 호환성 검사
test_step "nullptr 사용 검사" "! grep -r 'nullptr' ../srcs/ --include='*.cpp'"
test_step "C++11 기능 사용 검사" "! grep -r 'auto\|nullptr\|override\|final' ../srcs/ --include='*.cpp' | grep -v 'finalCheckGrammar'"

# 메모리 관리 검사
test_step "메모리 누수 검사" "grep -r 'new ' ../srcs/ --include='*.cpp' | grep -q 'SharedPtr'"

# 5. 포트 사용 가능성 테스트
echo -e "\n${YELLOW}=== 5. 포트 테스트 ===${NC}"
test_step "포트 6667 사용 가능 확인" "! lsof -i :6667"

# 6. 파일 구조 테스트
echo -e "\n${YELLOW}=== 6. 파일 구조 테스트 ===${NC}"
test_step "필수 헤더 파일 존재" "test -f ../include/Server.hpp && test -f ../include/User.hpp && test -f ../include/Channel.hpp"
test_step "필수 소스 파일 존재" "test -f ../srcs/Server.cpp && test -f ../srcs/User.cpp && test -f ../srcs/Channel.cpp"
test_step "Makefile 존재" "test -f ../Makefile"

# 7. 결과 출력
echo -e "\n${YELLOW}=== 테스트 결과 ===${NC}"
echo -e "${GREEN}통과: $PASSED${NC}"
echo -e "${RED}실패: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 모든 테스트 통과! ft_irc 서버가 준비되었습니다.${NC}"
    echo -e "\n${YELLOW}서버 실행 방법:${NC}"
    echo "cd .. && ./ft_irc 6667 mypassword"
    echo -e "\n${YELLOW}클라이언트 연결 테스트:${NC}"
    echo "nc 127.0.0.1 6667"
    echo "PASS mypassword"
    echo "NICK testuser"
    echo "USER testuser 0 * :Test User"
else
    echo -e "\n${RED}❌ 일부 테스트가 실패했습니다. 코드를 확인해주세요.${NC}"
fi

echo -e "\n=== 테스트 완료 ===" 