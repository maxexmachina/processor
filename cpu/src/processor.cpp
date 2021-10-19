#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../commands.h"
#include "../../config.h"
#include "../include/processor.h"
#include "../include/fileUtils.h"

void freeFileBuf(char *codeBuf) {
    if (codeBuf) {
        free(codeBuf - 4 - sizeof(unsigned int));
    }
} 

//TODO ASSERTS
int ProcessorInit(Processor *proc, const char *codePath) {
#if PROT_LEVEL > 0
    assert(proc);
#endif

    char *fileBuf = readFile(codePath, &proc->codeSize);
    if (fileBuf == nullptr) {
        return ERR_FILE_RD;
    }

    char tag[5] = "";
    memcpy(tag, fileBuf, 4);
    tag[4] = '\0';
    printf("Read tag %s\n", tag);
    if (strcmp(tag, TYPE_TAG) != 0) {
        printf("Binary file type tag %s doesn't match the required %s\n", tag, TYPE_TAG);
        free(fileBuf);
        return ERR_WRNG_TAG;
    }

    unsigned int *ver = (unsigned int *)(fileBuf + 4);
    printf("Read version %u\n", *ver);
    if (*ver != PROCESSOR_VER) {
        printf("Specified command set version %u doesn't match the processor version %u\n", *ver, PROCESSOR_VER);
        free(fileBuf);
        return ERR_WRNG_CMD_SET;
    }
    proc->code = fileBuf + 4 + sizeof(unsigned int);

    StackCtor(&proc->stack, sizeof(num_t), 10);
    proc->ip = 0;

    return 0;
}

int algebraicOperation(Stack *stack, AlgebraicOp op) {
#if PROT_LEVEL > 0
    assert(stack);
#endif
    elem_t lhs = 0;
    elem_t rhs = 0;
    int err = 0;
    StackPop(stack, &rhs, &err);
    if (err != 0) return err;
    StackPop(stack, &lhs, &err);
    if (err != 0) return err;
    elem_t res = 0;
    switch(op) {
        case OP_ADD:
            res = lhs + rhs;
            break;
        case OP_SUB:
            res = lhs - rhs;
            break;
        case OP_MUL:
            res = lhs * rhs; 
            break;
        case OP_DIV:
            if (rhs == 0) {
                printf("Cannot divide by zero\n");
                return ERR_DIV_ZERO;
            }
            res = lhs / rhs;
            break;
        default:
            printf("Undefined algebraic operation\n");
            return ERR_UNDEF_ALG_OP;
    }
    StackPush(stack, &res, &err);
    if (err != 0) return err;

    return 0;
}

int ProcessorRun(Processor *proc) {
#if PROT_LEVEL > 0
    assert(proc);
    assert(proc->code);
#endif
    while (proc->ip <= proc->codeSize) {
        switch(proc->code[proc->ip]) {
            case CMD_HLT:
                printf("Ending program run\n");
                StackDtor(&proc->stack);
                freeFileBuf(proc->code);
                return 0;
                break;
#if PROT_LEVEL > 0 
            case CMD_VER:
                ASSERT_OK(&proc->stack);
                ++proc->ip;
                break;
            case CMD_DMP:
                StackDump(&proc->stack, "DMP processor command");
                ++proc->ip;
                break;
#endif
            case CMD_OUT:
                {
                    elem_t topElem = 0;
                    StackTop(&proc->stack, &topElem);
                    printf("Stack top element : %lld\n", topElem);
                    ++proc->ip;
                    break;
                }
            case CMD_PUSH:
                StackPush(&proc->stack, proc->code + proc->ip + 1);
                proc->ip += 1 + sizeof(num_t);
                break;
            case CMD_POP:
                {
                    elem_t temp = 0;
                    StackPop(&proc->stack, &temp); 
                    ++proc->ip;
                    break;
                }
            case CMD_ADD:
                if (algebraicOperation(&proc->stack, OP_ADD) != 0) {
                    return ERR_ALG_FLR;
                } 
                ++proc->ip;
                break;
            case CMD_SUB:
                if (algebraicOperation(&proc->stack, OP_SUB) != 0) {
                    return ERR_ALG_FLR;
                } 
                ++proc->ip;
                break;
            case CMD_MUL:
                if (algebraicOperation(&proc->stack, OP_MUL) != 0) {
                    return ERR_ALG_FLR;
                } 
                ++proc->ip;
                break;
            case CMD_DIV:
                if (algebraicOperation(&proc->stack, OP_DIV) != 0) {
                    return ERR_ALG_FLR;
                } 
                ++proc->ip;
                break;
            default:
                printf("Undefined command\n");
                freeFileBuf(proc->code);
                return ERR_UNDEF_CMD;
        }
    }

    StackDtor(&proc->stack);
    freeFileBuf(proc->code);
    return 0;
}
