#ifndef PROCESSOR_H
#define PROCESSOR_H

const unsigned int PROCESSOR_VER = 1;

static const char *TYPE_TAG = "JEFF";

enum AlgebraicOp : int {
    ADD = 0,
    SUB = 1,
    MUL = 2,
    DIV = 3,
};

enum ProcessorError : int {
    ERR_FILE_RD = 1,
    ERR_WRNG_TAG = 2,
    ERR_WRNG_CMD_SET = 3,
    ERR_UNDEF_CMD = 4,
    ERR_DIV_ZERO = 5,
};

int run(const char *filePath);

#endif
