#include "../include/SHA256.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

std::string test_sha256(const std::string& str) {
    if (str.empty()) {
        return "";
    }
    
    SHA256 sha256;
    BYTE digest[SHA256_DIGEST_VALUELEN];
    sha256.SHA256_Encrpyt((const BYTE*)str.c_str(), str.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_VALUELEN; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    
    return ss.str();
}

void test_known_vectors() {
    std::cout << "=== SHA256 Known Test Vectors ===" << std::endl;
    
    // Test vector 1: empty string
    std::string result = test_sha256("");
    std::cout << "Empty string: " << result << std::endl;
    std::cout << "Expected:     e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" << std::endl;
    std::cout << "Match: " << (result == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" ? "YES" : "NO") << std::endl << std::endl;
    
    // Test vector 2: "abc"
    result = test_sha256("abc");
    std::cout << "\"abc\": " << result << std::endl;
    std::cout << "Expected: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" << std::endl;
    std::cout << "Match: " << (result == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" ? "YES" : "NO") << std::endl << std::endl;
    
    // Test vector 3: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
    result = test_sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    std::cout << "Long string: " << result << std::endl;
    std::cout << "Expected:    248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" << std::endl;
    std::cout << "Match: " << (result == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" ? "YES" : "NO") << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments - run test vectors
        test_known_vectors();
        return 0;
    }
    
    if (argc == 2) {
        std::string input = argv[1];
        
        // Check if it's a file or string
    }
    
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << argv[0] << "                    # Run test vectors" << std::endl;
    std::cout << "  " << argv[0] << " <file_or_string>  # Hash file or string" << std::endl;
    return 1;
}