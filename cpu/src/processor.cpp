#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../include/commands.h"
#include "../include/processor.h"
#include "../include/fileUtils.h"
#include "../include/stack.h"

int run(const char *filePath) {
    size_t bufSize = 0;
    char *fileBuf = readFile(filePath, &bufSize);
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
    char *codeBuf = fileBuf + 4 + sizeof(unsigned int);

    Stack stack = {};
    StackCtor(&stack, sizeof(num_t), 10); 
    size_t pc = 0;
    while (pc <= bufSize) {
        switch(codeBuf[pc]) {
            case CMD_HLT:
                printf("Ending program run\n");
                StackDtor(&stack);
                free(fileBuf);
                return 0;
                break;
#if DEBUG_MODE > 0 
            case CMD_VER:
                ASSERT_OK(&stack);
                ++pc;
                break;
            case CMD_DMP:
                StackDump(&stack, "DMP processor command");
                ++pc;
                break;
#endif
            case CMD_OUT:
                {
                    elem_t topElem = 0;
                    StackTop(&stack, &topElem);
                    printf("Stack top element : %lld\n", topElem);
                    ++pc;
                    break;
                }
            case CMD_PUSH:
                StackPush(&stack, codeBuf + pc + 1);
                pc += 1 + sizeof(num_t);
                break;
            case CMD_POP:
                {
                    elem_t temp = 0;
                    StackPop(&stack, &temp); 
                    ++pc;
                    break;
                }
            case CMD_ADD:
                {
                    elem_t lhs = 0;
                    elem_t rhs = 0;
                    StackPop(&stack, &rhs);
                    StackPop(&stack, &lhs);
                    elem_t res = lhs + rhs;
                    StackPush(&stack, &res); 
                    ++pc;
                    break;
                }
            case CMD_SUB:
                {
                    elem_t lhs = 0;
                    elem_t rhs = 0;
                    StackPop(&stack, &rhs);
                    StackPop(&stack, &lhs);
                    elem_t res = lhs + rhs;
                    StackPush(&stack, &res); 
                    ++pc;
                    break;
                }
            case CMD_MUL:
                {
                    elem_t lhs = 0;
                    elem_t rhs = 0;
                    StackPop(&stack, &rhs);
                    StackPop(&stack, &lhs);
                    elem_t res = lhs * rhs;
                    StackPush(&stack, &res); 
                    ++pc;
                    break;
                }
            case CMD_DIV:
                {
                    elem_t lhs = 0;
                    elem_t rhs = 0;
                    StackPop(&stack, &rhs);
                    if (rhs == 0) {
                        printf("Cannot divide by zero\n");
                        return ERR_DIV_ZERO;
                    }
                    StackPop(&stack, &lhs);
                    elem_t res = lhs * rhs;
                    StackPush(&stack, &res); 
                    ++pc;
                    break;
                }
                break;
            default:
                printf("Undefined command\n");
                free(fileBuf);
                return ERR_UNDEF_CMD;
        }
    }

    StackDtor(&stack);
    free(fileBuf);
    return 0;
}
