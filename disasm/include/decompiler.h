#ifndef DECOMPILER_H
#define DECOMPILER_H

#include <stdio.h>

#include "../../commands.h" 
#include "../../config.h"

const unsigned int DECOMPILER_VER = 1;

static const char *TYPE_TAG = "JEFF";

const unsigned char CMD_MASK = 0x1F;
const unsigned char MEM_MASK = 0x80;
const unsigned char REG_MASK = 0x40;
const unsigned char KONST_MASK = 0x20;

enum DecompilationError : int {
    ERR_FILE_RD = 1,
    ERR_WRNG_TAG = 2,
    ERR_WRNG_CMD_SET = 3,
    ERR_UNDEF_CMD = 4,
    ERR_DIV_ZERO = 5,
    ERR_ALG_FLR = 6,
    ERR_UNDEF_ALG_OP = 7,
    ERR_STK_ERR = 8,
    ERR_FILE_OPN = 9,
    ERR_FILE_WRT = 10,
    ERR_FILE_CLS = 11,
    ERR_GET_ARG = 12,
};

struct Registry {
    const char *name;
    const int id;
};

const Registry REG_MAP[] = {
    {.name = "ax", .id = 1},
    {.name = "bx", .id = 2},
    {.name = "cx", .id = 3},
    {.name = "dx", .id = 4},
};

const size_t N_REGS = sizeof(REG_MAP);

const char *getCmdName(Command cmd);

int writeCmd(FILE *fd, Command cmd, const char *arg=nullptr);

int decompile(const char *inPath, const char *outPath);

#endif
