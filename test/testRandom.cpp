#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

int main(void){

    int readByte = 16;
    std::fstream random("/dev/urandom", std::ios::in | std::ios::binary);
    if(!random){
        return(1);
    }

    char arr[18] = {0,};
    random.read(arr, readByte);
    if (!random) {
        std::cerr << "Error: Failed to read from /dev/urandom" << std::endl;
        return (1);
    }
    random.close();
    std::stringstream ss;
    for (int i = 0; i < readByte; i++) {
        // ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(static_cast<unsigned char>(arr[i]));
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(static_cast<unsigned char>(arr[i]));
    }
    std::cout << ss.str() << std::endl;
    return (0);
}