#include "../include/Password.hpp"

Password::Password(const Password &obj){
    (void)obj;
};

Password &Password::operator=(const Password &obj){
    (void)obj;
    return (*this);
};

Password::Password(): isPasswordSet(false), salt(""), hash("none"){

};

Password::~Password(){

};

bool Password::getIsPasswordSet() const{
    return this->isPasswordSet;
};

std::string const Password::getHash() const{
    return (this->hash);
};

void Password::setisPasswordSet(bool set){
    if(set == false){
        this->hash = "none";
    }
    this->hash = SHA256::SaltMaker();
    this->isPasswordSet = set;
};

//overload 고려 해야하나?
//안전한 사용을 위해서 기존 PassWord는 NULL로 초기화 해줍니다.
void Password::setPwd(std::string &Password){
    if (this->isPasswordSet == true){
        if (Password.empty()){
            std::cerr << "Error Retry" << std::endl;
            this->isPasswordSet = false;
            return;
        }
        this->hash = SHA256::SHA256Maker(Password + this->salt);
        Password = "";
    }
};

bool Password::CheckPassword(std::string &Password){
    if (this->isPasswordSet == false){
        std::cerr << "set Password" << std::endl;
        return (false);
    }
    std::string temp = SHA256::SHA256Maker(Password + this->salt);
    if (this->hash == temp){
        return (true);
    }
    return (false);
}
