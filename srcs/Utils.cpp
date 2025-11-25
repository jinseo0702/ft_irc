#include "../include/Utils.hpp"

std::string Utils::find_first_and_earse(const std::string &dst, const std::string &src){
    std::string temp = dst;
    size_t flag;

    flag = temp.find_first_not_of(src);
    if (flag != std::string::npos){
        temp.erase(0, flag);
    }
    return (temp);
};

bool Utils::is_nospcrlfcl(unsigned char c){
    return ((0x01 <= c && c <= 0x09) ||
            (0x0B <= c && c <= 0x0C) ||
            (0x0E <= c && c <= 0x1F) ||
            (0x21 <= c && c <= 0x39) ||
            (0x3B <= c && c <= 0xFF));
};



bool Utils::is_middle(const std::string &str){
    if (str.empty()){
        return false;
    }
    if (!is_nospcrlfcl(static_cast<unsigned char>(str[0]))){
        return false;
    }
    for (size_t i = 1; i < str.length(); ++i){
        if (!is_nospcrlfcl(static_cast<unsigned char>(str[i]))){
            return false;
        }
    }
    return true;
};



bool Utils::trailing(const std::string &str){
    if (str.empty()){
        return false;
    }
    for (size_t i = 0; i < str.length(); ++i){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (is_colon(uc) || is_space(uc) || is_nospcrlfcl(uc)){
            continue;
        }
        return false;
    }
    return true;
};


bool Utils::is_space(unsigned char c){
    return (c == 0x20);
};



bool Utils::is_crlf(unsigned char c){
    return (c == 0x0D || c == 0x0A);
};


bool Utils::is_target(const std::string &str){
    return (is_nickname(str) || is_servername(str));
};


bool Utils::is_msgto(const std::string &str){
    if (is_channel(str)){
        return (true);
    }
    else if (str.find('%')){
        std::stringstream is(str);
        std::string user;
        std::getline(is, user, '%');
        std::string host;
        std::getline(is, host);
        if ((is_user(user) && is_host(host))){
            return (true);
        }
    }
    else if (is_nickname(str)){
        return (true);
    }
    else if (str.find('!')){
        std::stringstream is(str);
        std::string nick;
        std::getline(is, nick, '!');
        std::string user;
        std::getline(is, user, '@');
        std::string host;
        std::getline(is, host);
        if ((is_nickname(nick) && is_user(user) && is_host(host))){
            return (true);
        }
    }
    return (false);
};



bool Utils::is_channel(const std::string &str){
    if (str.empty()){
        return (false);
    }
    unsigned char ucz = static_cast<unsigned char>(str[0]);
    if (!(ucz == '#'))
        return (false);
    std::stringstream is(str.substr(1));
    std::string temp;
    while (std::getline(is, temp, ':')){
        if (temp.empty()){
            return (false);
        }
        for (size_t i = 0; i < temp.length(); ++i){
            unsigned char uc = static_cast<unsigned char>(temp[i]);
            if (is_chanstring(uc) == true){
                continue;
            }
            return(false);
        }
    }
    return(true);
};



bool Utils::is_servername(const std::string &str){
    if (is_hostname(str)){
        return (true);
    }
    return (false);
};


bool Utils::is_host(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (!(is_hostname(str) || is_hostaddr(str))){
        return (false);
    }
    return (true);
};



bool Utils::is_hostname(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (str.length() > 63){
        return (false);
    }
    std::stringstream is(str);
    std::string temp;
    while (std::getline(is, temp, '.')){
        if (is_shortname(temp) == false){
            return (false);
        }
    }
    return (true);
};





bool Utils::is_shortname(const std::string &str){
    if (str.empty()){
        return (false);
    }
    unsigned char ucz = static_cast<unsigned char>(str[0]);
    if (!(is_letter(ucz) || is_digit(ucz))){
        return (false);
    }
    for (size_t i = 0; i < str.length(); ++i){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (ucz == '-' && uc == '-'){
            return (false);
        }
        ucz = uc;
        if (is_letter(uc))
            continue;
        if (is_digit(uc))
            continue;
        if (uc == '-')
            continue;
        return (false);
    }
    if(ucz == '-')
        return (false);
    return (true);
}




bool Utils::is_hostaddr(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (str.length() > 15){
        return (false);
    }

    int cntDot = std::count(str.begin(), str.end(), '.');
    if (cntDot != 3){
        return (false);
    }
    std::stringstream is(str);
    std::string temp;
    while (std::getline(is, temp, '.')){
        if (temp.length() > 3){
            return (false);
        }
        for (size_t i = 0; i < temp.length(); ++i){
            unsigned char uc = static_cast<unsigned char>(str[i]);
            if (is_digit(uc))
                continue;
            return (false);
        }
    }
    return (true);
};


bool Utils::is_nickname(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (str.length() > 8){
        return (false);
    }
    unsigned char ucz = static_cast<unsigned char>(str[0]);
    if (!(is_letter(ucz) || is_special(ucz))){
        return (false);
    }
    for (size_t i = 0; i < str.length(); ++i){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (is_letter(uc))
            continue;
        if (is_digit(uc))
            continue;
        if (is_special(uc))
            continue;
        if (uc == '-')
            continue;
        return (false);
    }
    return (true);
};




bool Utils::is_chanstring(unsigned char c){
    if( (0x01 <= c && c <= 0x07) ||
        (0x08 <= c && c <= 0x09) ||
        (0x0B <= c && c <= 0x0C) ||
        (0x0E <= c && c <= 0x1F) ||
        (0x21 <= c && c <= 0x2B) ||
        (0x2D <= c && c <= 0x39) ||
        (0x3B <= c && c <= 0xFF) ){
        return (true);
    }
    return (false);
};




bool Utils::is_user(const std::string &str){
    if (str.empty()){
        return (false);
    }
    for (size_t i = 0; i < str.length(); ++i){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (0x01 <= uc && uc <= 0x09)
            continue;
        if (0x0B <= uc && uc <= 0x0C)
            continue;
        if (0x0E <= uc && uc <= 0x1F)
            continue;
        if (0x21 <= uc && uc <= 0x3F)
            continue;
        if (0x41 <= uc && uc <= 0xFF)
            continue;
        return (false);
    }
    return (true);
};





bool Utils::is_key(const std::string &str){
    if (str.empty() || str.length() > 23){
        return (false);
    }
    for (size_t i = 0; i < str.length(); ++i){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (0x01 <= uc && uc <= 0x05)
            continue;
        if (0x07 <= uc && uc <= 0x08)
            continue;
        if (0x0C == uc)
            continue;
        if (0x0E <= uc && uc <= 0x1F)
            continue;
        if (0x21 <= uc && uc <= 0x7F)
            continue;
        return (false);
    }
    return (true);
};


bool Utils::is_letter(unsigned char c){
    if ((0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A)){
        return (true);
    }
    return (false);
};


bool Utils::is_digit(unsigned char c){
    if ((0x30 <= c && c <= 0x39)){
        return (true);
    }
    return (false);
};


bool Utils::is_special(unsigned char c){
    if ((0x5B <= c && c <= 0x60) || (0x7B <= c && c <= 0x7D)){
        return (true);
    }
    return (false);
};

bool Utils::is_colon(unsigned char c){
    if (0x3A == c){
        return (true);
    }
    return (false);
};

