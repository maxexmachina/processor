#include <cerrno>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../../include/fileUtils.h"
#include "decompiler.h"
#include "../../commands.h"

int writeCmd(FILE *fd, Command cmd, const char *arg) {
    assert(fd);
    assert(arg);

    if (fprintf(fd, "%s ", getCmdName(cmd)) < 0) {
        return ERR_FILE_WRT;
    }
    if (arg) {
        if (fprintf(fd, "%s\n", arg) < 0) {
            return ERR_FILE_WRT;
        }
    } else {
        if (fprintf(fd, "\n") < 0) {
            return ERR_FILE_WRT;
        }
    } 

    return 0;
}

const char *getCmdName(Command cmd) {
    for (size_t i = 0; i < CMD_SET_LEN; ++i) {
        if (CMD_NAME_MAP[i].id == cmd) {
            return CMD_NAME_MAP[i].name;
        }
    }
    return nullptr;
}

char *getRegName(int id) {
    for (size_t i = 0; i < N_REGS; ++i) {
        if (id == REG_MAP[i].id) {
            return (char *)REG_MAP[i].name;
        }
    }

    return 0;
}

int getArgStr(char *argStr, char *code, size_t *ip, int cmd, int type) {
    assert(argStr);
    assert(code);
    assert(ip);

    char reg[8] = "";
    char num[64] = "";
    switch(cmd) {
        case CMD_PUSH:
            {
                if (type & KONST_MASK) {
                    sprintf(num, "%lld", *(num_t *)(code + *ip)); 
                    *ip += sizeof(num_t);
                }
                if (type & REG_MASK) {
                    sprintf(reg, "%s", code + *(ip++));
                }
                if (strcmp(reg, "") != 0 && strcmp(num, "") != 0) {
                    sprintf(argStr, "%s+%s", reg, num); 
                } else if (strcmp(reg, "") != 0) {
                    sprintf(argStr, "%s", reg);
                } else if (strcmp(num, "") != 0) {
                    sprintf(argStr, "%s", num);
                } 
            }
            break;
        case CMD_POP:
            if (type & REG_MASK) {
                sprintf(argStr, "%s", code + *(ip++));
            }
            break;
        default:
            printf("This command isn't supposted to have arguments\n");
            return 1;
    }

    return 0;
}

int decompile(const char *inPath, const char *outPath) {
    assert(inPath);
    assert(outPath);

    size_t bufSize;
    char *fileBuf = readFile(inPath, &bufSize);
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
    if (*ver != DECOMPILER_VER) {
        printf("Specified command set version %u doesn't match the decompiler version %u\n", *ver, DECOMPILER_VER);
        free(fileBuf);
        return ERR_WRNG_CMD_SET;
    }
    char *codeBuf = fileBuf + 4 + sizeof(unsigned int);
    bufSize -= 4 + sizeof(unsigned int);

    FILE *outFile = fopen(outPath, "w");
    if (outFile == nullptr) {
        printf("Error opening file %s : %s\n", outPath, strerror(errno));
        free(fileBuf);
        return ERR_FILE_OPN;
    }

    size_t ip = 0;
    while (ip < bufSize) {

        int cmd = codeBuf[ip] & CMD_MASK;
        printf("Command : %x\n", cmd);
        int type = codeBuf[ip];
        printf("type : %x\n", type);
        ++ip;
        
        char *argStr = (char *)calloc(64, sizeof(*argStr));
        int ret = getArgStr(argStr, codeBuf, &ip, cmd, type); 
        if (ret != 0) {
            free(argStr);
            return ERR_GET_ARG;
        }

        switch(cmd) {
            case CMD_HLT:
                writeCmd(outFile, CMD_HLT);
                break;
#if PROT_LEVEL > 0 
            case CMD_VER:
                writeCmd(outFile, CMD_VER);
                break;
            case CMD_DMP:
                writeCmd(outFile, CMD_DMP);
                break;
#endif
            case CMD_OUT:
                writeCmd(outFile, CMD_OUT);
                break;
            case CMD_PUSH:
                writeCmd(outFile, CMD_PUSH, argStr);
                break;
            case CMD_POP:
                writeCmd(outFile, CMD_POP, argStr);
                break;
            case CMD_ADD:
                writeCmd(outFile, CMD_ADD);
                break;
            case CMD_SUB:
                writeCmd(outFile, CMD_SUB);
                break;
            case CMD_MUL:
                writeCmd(outFile, CMD_MUL);
                break;
            case CMD_DIV:
                writeCmd(outFile, CMD_DIV);
                break;
            default:
                printf("Undefined command\n");
                free(fileBuf);
                fclose(outFile);
                return ERR_UNDEF_CMD;
        }
        free(argStr);
    }

    free(fileBuf);
    if (fclose(outFile) == EOF) {
        return ERR_FILE_CLS;
    }

    return 0;
}
