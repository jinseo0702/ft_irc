#include "./KISA_SHA256.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>  // 추가
#include <string>   // 추가

std::string sha256_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return "";
    
    SHA256_INFO info;
    SHA256_Init(&info);
    
    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        SHA256_Process(&info, (const BYTE*)buffer, file.gcount());
    }
    
    BYTE digest[SHA256_DIGEST_VALUELEN];
    SHA256_Close(&info, digest);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_VALUELEN; i++) {
        // ss << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
        ss << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    
    return ss.str();
}

std::string test_sha256(const std::string& str) {
    if (str.empty()){
        return "";
    }
    
    BYTE digest[SHA256_DIGEST_VALUELEN];
    SHA256_Encrpyt((const BYTE*)str.c_str(), str.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_VALUELEN; i++) {
        // ss << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
        // ss << std::setfill('0') << std::setw(2) << (int)digest[i];
        // ss << std::setw(2) << (int)digest[i];
        // ss << std::hex << (int)digest[i];
        // ss << (int)digest[i];
        // ss << std::hex << std::setw(2) << (int)digest[i];
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    std::string hash = test_sha256(filename);
    
    if (hash.empty()) {
        std::cout << "Error: Could not read file " << filename << std::endl;
        return 1;
    }
    
    std::cout << "SHA256" << " = " << hash << std::endl;
    return 0;
}