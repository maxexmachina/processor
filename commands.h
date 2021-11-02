#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

const unsigned int CMD_SET_VERSION = 1;

typedef long long num_t;

//TODO in command
enum Command : int {
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

struct Cmd {
    short id;
    const char *name;
};

const Cmd CMD_NAME_MAP[] = {
    {.id = 0, .name = "hlt"},
    {.id = 1, .name = "ver"},
    {.id = 2, .name = "dmp"},
    {.id = 3, .name = "out"},
    {.id = 4, .name = "push"},
    {.id = 5, .name = "pop"},
    {.id = 6, .name = "add"},
    {.id = 7, .name = "sub"},
    {.id = 8, .name = "mul"},
    {.id = 9, .name = "div"},
}; 

const size_t CMD_SET_LEN = sizeof(CMD_NAME_MAP);

#endif
