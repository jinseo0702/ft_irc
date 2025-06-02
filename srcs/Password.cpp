#include "../include/Password.hpp"

Password::Password(const Password &obj){
    (void)obj;
};

Password &Password::operator=(const Password &obj){
    (void)obj;
    return (*this);
};

Password::Password(): isPasswordSet(false), pwd(-999){

};

Password::~Password(){

};

bool const Password::getIsPassworldSet() const{
    return (this->isPasswordSet);
};

int const Password::getPwd() const{
    return (this->pwd);
};

void Password::setisPasswordSet(bool set){
    if(set == false){
        this->pwd = -999;
    }
    this->isPasswordSet = set;
};

//overload 고려 해야하나?
void Password::setPwd(int pwd){
    if (this->isPasswordSet == true){
        if (pwd < 0){
            std::cerr << "Error Retry" << std::endl;
            this->isPasswordSet = false;
            return;
        }
        this->pwd = pwd;
    }
};
