#ifndef DECOMPILER_H
#define DECOMPILER_H

#include <stdio.h>

#include "../../commands.h" 
#include "../../config.h"

const unsigned int DECOMPILER_VER = 1;

static const char *TYPE_TAG = "JEFF";

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
};

const char *getCmdName(Command cmd);

int writeCmd(FILE *fd, Command cmd, const char *arg=nullptr);

int decompile(const char *inPath, const char *outPath);

#endif
