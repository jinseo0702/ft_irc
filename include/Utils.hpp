#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <list>
#include <cctype>

class Utils {
    public:
    static std::string find_first_and_earse(const std::string &dst, const std::string &src);

    static bool is_middle(const std::string &str);
    static bool trailing(const std::string &str);
    static bool is_nospcrlfcl(unsigned char c);
    static bool is_space(unsigned char c);
    static bool is_crlf(unsigned char c);

    static bool is_target(const std::string &str);
    static bool is_msgto(const std::string &str);
    static bool is_channel(const std::string &str);
    static bool is_servername(const std::string &str);
    static bool is_host(const std::string &str);
    static bool is_hostname(const std::string &str);
    static bool is_shortname(const std::string &str);
    static bool is_hostaddr(const std::string &str);
    static bool is_nickname(const std::string &str);
    static bool is_chanstring(unsigned char c);
    static bool is_user(const std::string &str);
    static bool is_key(const std::string &str);
    static bool is_letter(unsigned char c);
    static bool is_digit(unsigned char c);
    static bool is_special(unsigned char c);
    static bool is_colon(unsigned char c);
};

#endif