#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

const unsigned int CMD_SET_VERSION = 3;

typedef long num_t;
const size_t NUM_LEN = sizeof(num_t);

const unsigned int FX_POINT_PRECISION = 1000; 
const double EPSILON = 0.1;

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
    CMD_DIV = 9,
	CMD_JMP = 10,
	CMD_JA = 11,
	CMD_JAE = 12,
	CMD_JB = 13,
	CMD_JBE = 14,
	CMD_JE = 15,
	CMD_JNE = 16,
	CMD_JF = 17,
	CMD_CALL = 18,
	CMD_RET = 19,
	CMD_IN = 20,
	CMD_ABS = 21,
	CMD_SQRT = 22,
	CMD_DRAW = 23,
	CMD_SLEEP = 24,
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
    {.id = 10, .name = "jmp"},
    {.id = 11, .name = "ja"},
    {.id = 12, .name = "jae"},
    {.id = 13, .name = "jb"},
    {.id = 14, .name = "jbe"},
    {.id = 15, .name = "je"},
    {.id = 16, .name = "jne"},
    {.id = 17, .name = "jf"},
    {.id = 18, .name = "call"},
    {.id = 19, .name = "ret"},
    {.id = 20, .name = "in"},
    {.id = 21, .name = "abs"},
    {.id = 22, .name = "sqrt"},
    {.id = 23, .name = "draw"},
    {.id = 24, .name = "sleep"},
}; 

const size_t CMD_SET_LEN = sizeof(CMD_NAME_MAP) / sizeof(*CMD_NAME_MAP);

#endif
