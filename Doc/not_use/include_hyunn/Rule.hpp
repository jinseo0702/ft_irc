#ifndef RULE_HPP
#define RULE_HPP

typedef enum oper_role{
    KICK = 0x001,
    INVITE = 0x002,
    TOPIC = 0x003,
    MODE = 0x004,
};

typedef enum mode_option{
    MODE_INVITE = 0x001,
    MODE_TOPIC = 0x002,
    MODE_KEY = 0x003,
    MODE_OWNER = 0x004,
    MODE_LIMIT = 0x005,
};

typedef enum option_state{
    OPTION_ON = 0x001,
    OPTION_OFF = 0x002,
};

#endif