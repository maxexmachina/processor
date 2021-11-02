#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "../../include/stack.h"
#include "../../commands.h"

const unsigned int PROCESSOR_VER = 2;

static const char TYPE_TAG[] = "JEFF";
const size_t TYPE_TAG_LEN = sizeof(TYPE_TAG) - 1;

typedef unsigned int ver_t;
const size_t VER_LEN = sizeof(ver_t);

const size_t RAM_SIZE = 1048576;

const size_t N_REGS = 4;

struct Processor {
    Stack stack;
    char *code;
    size_t codeSize;
    size_t ip;
    num_t regs[N_REGS];
	char *ram;
};

const unsigned char CMD_MASK = 0x1F;
const unsigned char RAM_MASK = 0x80;
const unsigned char REG_MASK = 0x40;
const unsigned char KONST_MASK = 0x20;

enum AlgebraicOp : int {
    OP_ADD = 0,
    OP_SUB = 1,
    OP_MUL = 2,
    OP_DIV = 3,
};

enum ProcessorError : int {
    ERR_FILE_RD = 1,
    ERR_WRNG_TAG = 2,
    ERR_WRNG_CMD_SET = 3,
    ERR_UNDEF_CMD = 4,
    ERR_DIV_ZERO = 5,
    ERR_ALG_FLR = 6,
    ERR_UNDEF_ALG_OP = 7,
    ERR_STK_ERR = 8,
    ERR_WRNG_REG = 9,
    ERR_STK_POP = 10,
    ERR_STK_PUSH = 11,
	ERR_NOMEM = 12,
	ERR_WRNG_ARG = 13,
};

num_t getArg(Processor *proc, int cmd, int type);

const char *getCmdName(int cmd);

int ProcessorRun(Processor *proc);

int ProcessorInit(Processor *proc, const char *codePath);

void freeCpu(Processor *proc);

int algebraicOperation(Stack *stack, AlgebraicOp op);

#endif
