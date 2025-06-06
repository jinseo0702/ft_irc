#ifndef PASSWORD_HPP
#define PASSWORD_HPP

#include <iostream>

class Password {
    private:
        bool isPasswordSet;
        int pwd;
        Password(const Password &obj);
        Password &operator=(const Password &obj);
    public:
        Password();
        ~Password();
        // Getter
        bool const getIsPasswordSet() const;
        int const getPwd() const;
        // Setter
        void setisPasswordSet(bool set);
        void setPwd(int pwd);
};

#endif