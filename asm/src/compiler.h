#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

#include "../../include/fileUtils.h"
#include "../../commands.h"

static const char TYPE_TAG[] = "JEFF";
const size_t TYPE_TAG_LEN = sizeof(TYPE_TAG) - 1;

typedef unsigned int ver_t;
const size_t VER_LEN = sizeof(ver_t);

const size_t MAX_LINE_LEN = 64;
const size_t MAX_LABEL_LEN = 16;
const size_t NUM_LABELS = 64;

struct label {
	size_t addr;
	char name[MAX_LABEL_LEN];
};

const size_t MAX_CMD_LEN = 16;

struct command_t {
    char cmd[MAX_CMD_LEN];
	size_t numArgs;
    bool hasKonst;
    num_t konst;
    bool hasReg;
	int reg;
	bool hasRam;
	bool hasLabel;
	char label[MAX_LABEL_LEN];
};

const unsigned char RAM_BIT = 0x80;
const unsigned char REG_BIT = 0x40;
const unsigned char KONST_BIT = 0x20;

struct Registry {
    const char *name;
    const int id;
};

const Registry REG_MAP[] = {
    {.name = "ax", .id = 1},
    {.name = "bx", .id = 2},
    {.name = "cx", .id = 3},
    {.name = "dx", .id = 4},
    {.name = "ex", .id = 5},
    {.name = "fx", .id = 6},
    {.name = "gx", .id = 7},
    {.name = "hx", .id = 8},
};

const size_t N_REGS = sizeof(REG_MAP) / sizeof(*REG_MAP);

const size_t MAX_CMD_ARR_LEN = 10000000000;

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


int getCommand(const char *textLine, command_t *curCommand);

int getRegId(char first);

int compile(const char *inPath, const char *outPath=nullptr);

int printCompilationError(int errCode, size_t lineNum, const char *filePath,
        void *commandArray=nullptr, FILE *compiled=nullptr, text_t *text=nullptr);

#endif
