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
    if( (0x01 <= c && c <= 0x09) ||
        (0x0B <= c && c <= 0x0C) ||
        (0x0E <= c && c <= 0x1F) ||
        (0x21 <= c && c <= 0x39) ||
        (0x3B <= c && c <= 0xFF) ){
        return (true);
    }
    return (false);
};



//middle     =  nospcrlfcl *( ":" / nospcrlfcl )
//nospcrlfcl 1개 *( ":" / nospcrlfcl ) 은 0 ~ 무한개
bool Utils::is_middle(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (is_nospcrlfcl(static_cast<unsigned char>(str[0])) == false){
        return (false);
    }
    for (int i = 0; i < str.length(); i++){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (is_nospcrlfcl(uc) == false){
            return(false);
        }
    };
    return (true);
};

//trailing   =  *( ":" / " " / nospcrlfcl ) 0 ~ 무한 일단 있다고 가정하고 계산하겠습니다.
//trailing 은 문자하나당 하나만 존재 합니다.
bool Utils::trailing(const std::string &str){
    if (str.empty()){
        return (false);
    }
    for (int i = 0; i < str.length(); i++){
        unsigned char uc = static_cast<unsigned char>(str[i]);
        if (is_colon(uc))
            continue;
        if (is_space(uc))
            continue;
        if (is_nospcrlfcl(uc))
            continue;
        return (false);
    };
    return (true);
};

//SPACE      =  %x20        ; space character
bool Utils::is_space(unsigned char c){
    if (c == 0x20)
        return (true);
    return (false);
};

//crlf       =  %x0D %x0A   ; "carriage return" "linefeed"
//이건 문자열일까 문자일까??
bool Utils::is_crlf(unsigned char c){
    if (c == 0x0D || c == 0x0A){
        return (true);
    }
    return (false);   
};

//target     =  nickname / server
bool Utils::is_target(const std::string &str){
    if (!(is_nickname(str) || is_servername(str))){
        return (false);
    }
    return (true);
};

//  msgto     = channel / ( user "%" host ) / nickname / ( nickname "!" user "@" host )
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

// channel    =  ( "#" ) chanstring [ ":" chanstring ]
// ; 채널을 표현할 수있는 방법은 ( "#" / "+" / ( "!" channelid ) / "&" ) 아닌 '#'으로 통일 하겠습니다.
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
        for (int i = 0; i < temp.length(); i++){
            unsigned char uc = static_cast<unsigned char>(temp[i]);
            if (is_chanstring(uc) == true){
                continue;
            }
            return(false);
        }
    }
    return(true);
};


//servername =  hostname
bool Utils::is_servername(const std::string &str){
    if (is_hostname(str)){
        return (true);
    }
    return (false);
};

// host       =  hostname / hostaddr
bool Utils::is_host(const std::string &str){
    if (str.empty()){
        return (false);
    }
    if (!(is_hostname(str) || is_hostaddr(str))){
        return (false);
    }
    return (true);
};

// hostname   =  shortname *( "." shortname )
// hostname 의 최대 길이는 63자입니다.
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

// shortname  =  ( letter / digit ) *( letter / digit / "-" )
// *( letter / digit )
//   ; as specified in RFC 1123 [HNAME]
// continuous hypen is failed;
bool Utils::is_shortname(const std::string &str){
    if (str.empty()){
        return (false);
    }
    unsigned char ucz = static_cast<unsigned char>(str[0]);
    if (!(is_letter(ucz) || is_digit(ucz))){
        return (false);
    }
    for (int i = 0; i < str.length(); i++){
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


// hostaddr   =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
// ; ip4addr
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
        for (int i = 0; i < temp.length(); i++){
            unsigned char uc = static_cast<unsigned char>(str[i]);
            if (is_digit(uc))
                continue;
            return (false);
        }
    }
    return (true);
};

// nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
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
    for (int i = 0; i < str.length(); i++){
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

// chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
// chanstring =/ %x2D-39 / %x3B-FF
//; any octet except NUL, BELL, CR, LF, " ", "," and ":"
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

//user =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
//; any octet except NUL, CR, LF, " " and "@"
//umm..... maybe think Korean????
bool Utils::is_user(const std::string &str){
    if (str.empty()){
        return (false);
    }
    for (int i = 0; i < str.length(); i++){
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


// key        =  1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
// ; any 7-bit US_ASCII character,
// ; except NUL, CR, LF, FF, h/v TABs, and " "
bool Utils::is_key(const std::string &str){
    if (str.empty() || str.length() > 23){
        return (false);
    }
    for (int i = 0; i < str.length(); i++){
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

// letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
bool Utils::is_letter(unsigned char c){
    if ((0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A)){
        return (true);
    }
    return (false);
};

// digit      =  %x30-39                 ; 0-9
bool Utils::is_digit(unsigned char c){
    if ((0x30 <= c && c <= 0x39)){
        return (true);
    }
    return (false);
};

// ; "[", "]", "\", "`", "_", "^", "{", "|", "}"
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

