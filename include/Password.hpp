#ifndef PASSWORD_HPP
#define PASSWORD_HPP

#include <iostream>
#include <string>
#include "./SHA256.hpp"

class Password {
    private:
        bool isPasswordSet;
        std::string salt;
        std::string hash;
        Password(const Password &obj);
        Password &operator=(const Password &obj);
    public:
        Password();
        ~Password();
        // Getter
        bool const getIsPasswordSet() const;
        std::string const getHash() const;
        // Setter
        void setisPasswordSet(bool set);
        void setPwd(std::string &Password);

        bool CheckPassword(std::string &Password);
};

#endif