#include <iostream>
#include <fstream>
#include <string>
#include "../include/DCC.hpp"

int main() {
    std::cout << "=== DCC 테스트 ===" << std::endl;
    
    // 테스트 파일 생성
    std::ofstream testFile("test_file.txt");
    testFile << "This is a test file for DCC transfer." << std::endl;
    testFile << "Line 2 of the test file." << std::endl;
    testFile << "Line 3 of the test file." << std::endl;
    testFile.close();
    
    std::cout << "테스트 파일 'test_file.txt' 생성 완료" << std::endl;
    
    // DCC 매니저 테스트
    DCCManager dccManager;
    
    std::cout << "DCC 매니저 초기화 완료" << std::endl;
    std::cout << "업로드 디렉토리: " << dccManager.getUploadDirectory() << std::endl;
    
    // 세션 생성 테스트
    int sessionId = dccManager.createSession(DCCSession::SEND, "test_file.txt", 
                                           "sender", "receiver", 1024);
    
    if (sessionId > 0) {
        std::cout << "DCC 세션 생성 성공: ID = " << sessionId << std::endl;
        
        DCCSession* session = dccManager.getSession(sessionId);
        if (session) {
            std::cout << "세션 정보:" << std::endl;
            std::cout << "  파일명: " << session->getFilename() << std::endl;
            std::cout << "  발신자: " << session->getSender() << std::endl;
            std::cout << "  수신자: " << session->getReceiver() << std::endl;
            std::cout << "  포트: " << session->getPort() << std::endl;
            std::cout << "  IP: " << session->getIp() << std::endl;
            std::cout << "  파일 크기: " << session->getFileSize() << std::endl;
        }
        
        // 세션 제거
        dccManager.removeSession(sessionId);
        std::cout << "DCC 세션 제거 완료" << std::endl;
    } else {
        std::cout << "DCC 세션 생성 실패" << std::endl;
    }
    
    std::cout << "=== DCC 테스트 완료 ===" << std::endl;
    return 0;
} 