#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "stack.h"
#include "../../commands.h"

const unsigned int PROCESSOR_VER = 1;

static const char *TYPE_TAG = "JEFF";

struct Processor {
    Stack stack;
    char *code;
    size_t codeSize;
    size_t ip;
    num_t regs[4];
};

const char CMD_MASK = 0x1F;
const char MEM_MASK = 0x80;
const char REG_MASK = 0x40;
const char KONST_MASK = 0x20;

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
};

int ProcessorRun(Processor *proc);

int ProcessorInit(Processor *proc, const char *codePath);

void freeFileBuf(char *codeBuf);

int algebraicOperation(Stack *stack, AlgebraicOp op);

#endif
