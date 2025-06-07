#include "../include/Parser.hpp" // 실제 Parser 클래스 헤더 파일 경로에 맞게 수정하세요.
#include <iostream>

// 제공된 Parser 클래스 구현이 별도 파일(예: Parser.cpp)에 있다면 컴파일 시 함께 포함해야 합니다.
// 만약 헤더에 모든 구현이 있다면 이 파일만으로 충분합니다.

int main(int argc, char **argv)
{
    // 1. 프로그램 실행 시 인자가 없으면 사용법을 안내하고 종료합니다.
    if (argc != 2)
    {
        std::cerr << "Usage: ./test_parser \":prefix command param1 param2\"" << std::endl;
        return (1);
    }

    // 2. 두 번째 인자(테스트할 문자열)를 파싱합니다.
    std::string line_to_parse = argv[1];
    Parser p = Parser::parse(line_to_parse);

    // 3. 파싱 결과를 확인하고 표준 출력으로 인쇄합니다.
    //    이 출력 형식을 파이썬에서 그대로 검증하게 됩니다.
    // if (p.isValid())
    // {
        std::cout << p; // 오버로딩된 << 연산자가 prefix, command, params를 출력합니다.
    // }
    // else
    // {
    //     std::cout << "INVALID" << std::endl;
    // }

    return (0);
}