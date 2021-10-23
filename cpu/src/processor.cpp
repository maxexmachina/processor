#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../commands.h"
#include "../../config.h"
#include "processor.h"
#include "../../include/fileUtils.h"

void freeFileBuf(char *codeBuf) {
    assert(codeBuf);
    free(codeBuf - 4 - sizeof(unsigned int));
} 

const char *getCmdName(int cmd) {
    for (size_t i = 0; i < CMD_SET_LEN; ++i) {
        if (CMD_NAME_MAP[i].id == cmd) {
            return CMD_NAME_MAP[i].name;
        }
    }
    return nullptr;
}

int ProcessorInit(Processor *proc, const char *codePath) {
    assert(proc);
    assert(codePath);

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
    proc->codeSize -= 4 + sizeof(unsigned int);

    StackCtor(&proc->stack, sizeof(num_t), 10);
    proc->ip = 0;

    return 0;
}

int algebraicOperation(Stack *stack, AlgebraicOp op) {
    assert(stack);

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

num_t getArg(Processor *proc, int cmd, int type) {
    assert(proc);

    num_t arg = 0;
    switch(cmd) {
        case CMD_PUSH:
            {
                if (type & KONST_MASK) {
                    arg += *(num_t *)(proc->code + proc->ip);
                    proc->ip += sizeof(num_t);
                }
                if (type & REG_MASK) arg += proc->regs[proc->code[proc->ip++] - 1];
            }
            break;
        case CMD_POP:
            if (type & REG_MASK) arg = proc->code[proc->ip++];
            break;
        default:
            printf("This command isn't supposted to have arguments\n");
    }
    return arg;
}

int ProcessorRun(Processor *proc) {
    assert(proc);
    assert(proc->code);

    while (proc->ip < proc->codeSize) {
        int cmd = proc->code[proc->ip] & CMD_MASK;
        int type = proc->code[proc->ip];
        ++proc->ip;

        switch(cmd) {
            case CMD_HLT:
                printf("Ending program run\n");
                StackDtor(&proc->stack);
                freeFileBuf(proc->code);
                return 0;
                break;
#if PROT_LEVEL > 0 
            case CMD_VER:
                ASSERT_OK(&proc->stack);
                break;
            case CMD_DMP:
                StackDump(&proc->stack, "DMP processor command");
                break;
#endif
            case CMD_OUT:
                {
                    elem_t topElem = 0;
                    int err = 0;
                    StackTop(&proc->stack, &topElem, &err);
                    if (!err) { 
                        printf("Stack top element : %lld\n", topElem);
                    } else if (err == STK_UNDERFL) {
                        printf("No elements on the stack\n");
                    } else {
                        printf("Undefined error in StackTop\n");
                        freeFileBuf(proc->code);
                        return ERR_STK_TOP;
                    }
                    break;
                }
            case CMD_PUSH:
                {
                    num_t arg = getArg(proc, cmd, type);
                    int err = 0;
                    StackPush(&proc->stack, &arg, &err);
                    if (err) {
                        printf("Error in StackPush\n");
                        freeFileBuf(proc->code);
                        return ERR_STK_PUSH;
                    }
                    break;
                }
            case CMD_POP:
                {
                    num_t arg = getArg(proc, cmd, type);
                    if (arg == 0) {
                        elem_t temp = 0;
                        StackPop(&proc->stack, &temp); 
                    } else if (arg > 0 && arg < 5) {
                        StackPop(&proc->stack, &proc->regs[arg - 1]);
                    } else {
                        printf("Wrong register number in pop command\n");
                        freeFileBuf(proc->code);
                        return ERR_WRNG_REG; 
                    }
                    break;
                }
            case CMD_ADD:
                if (algebraicOperation(&proc->stack, OP_ADD) != 0) {
                    return ERR_ALG_FLR;
                } 
                break;
            case CMD_SUB:
                if (algebraicOperation(&proc->stack, OP_SUB) != 0) {
                    return ERR_ALG_FLR;
                } 
                break;
            case CMD_MUL:
                if (algebraicOperation(&proc->stack, OP_MUL) != 0) {
                    return ERR_ALG_FLR;
                } 
                break;
            case CMD_DIV:
                if (algebraicOperation(&proc->stack, OP_DIV) != 0) {
                    return ERR_ALG_FLR;
                } 
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
