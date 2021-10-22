#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

#include "../include/fileUtils.h"

struct command {
    char cmd[16];
    bool hasKonst;
    long long int konst;
    bool hasReg;
    char reg[4];
};

const unsigned char RAM_BIT = 0x80;
const unsigned char REG_BIT = 0x40;
const unsigned char KONST_BIT = 0x20;

struct Registry {
    const char *name;
    const int id;
};

const Registry regMap[4] = {
    {.name = "ax", .id = 1},
    {.name = "bx", .id = 2},
    {.name = "cx", .id = 3},
    {.name = "dx", .id = 4},
};

typedef long long num_t;

const int MAX_CMD_ARR_LEN = 4096;

enum CompilationError : int {
    ERR_NOMEM = 1,
    ERR_FILE_OPN = 2,
    ERR_FILE_CLS = 3,
    ERR_CMD_SCAN = 4,
    ERR_ARG_COUNT = 5,
    ERR_FILE_WRT = 6,
    ERR_UNDEF_CMD = 7,
    ERR_CMD_BUFF_LEN = 8,
    ERR_WRNG_ARG = 10,
};

int getCommand(const char *textLine, command *curCommand, size_t *nArgs);

int getRegId(const char *name);

int compile(const char *inPath, const char *outPath=nullptr);

int printCompilationError(int errCode, size_t lineNum, const char *filePath,
        void *commandArray=nullptr, FILE *compiled=nullptr, text_t *text=nullptr);

#endif
