#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

const unsigned int CMD_SET_VERSION = 1;

enum Commands : int {
    CMD_HLT = 0,
    CMD_VER = 1,
    CMD_DMP = 2,
    CMD_OUT = 3,
    CMD_PUSH = 4,
    CMD_POP = 5,
    CMD_ADD = 6,
    CMD_SUB = 7,
    CMD_MUL = 8,
    CMD_DIV = 9
};

#endif
