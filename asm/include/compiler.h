#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

#include "../include/fileUtils.h"

struct command {
    const char text[16];
    long long int num;
};

typedef long long num_t;

const int MAX_CMD_ARR_LEN = 4096 * 100;

enum CompilationError : int {
    ERR_NOMEM = 1,
    ERR_FILE_OPN = 2,
    ERR_FILE_CLS = 3,
    ERR_CMD_SCAN = 4,
    ERR_ARG_COUNT = 5,
    ERR_FILE_WRT = 6,
    ERR_UNDEF_CMD = 7,
    ERR_CMD_BUFF_LEN = 8,
};

int compile(const char *inPath, const char *outPath=nullptr);

int printCompilationError(int errCode, size_t line, const char *filePath,
        void *commandArray=nullptr, FILE *compiled=nullptr, text_t *text=nullptr);

#endif
