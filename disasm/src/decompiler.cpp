#include <cerrno>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/fileUtils.h"
#include "../include/decompiler.h"
#include "../../commands.h"

int writeCmd(FILE *fd, Command cmd, const char *arg) {
#if PROT_LEVEL > 0
    assert(cmd)
#endif

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
    for (size_t i = 0; i < cmdSetLen; ++i) {
        if (cmdNameMap[i].id == cmd) {
            return cmdNameMap[i].name;
        }
    }
    return nullptr;
}

int decompile(const char *inPath, const char *outPath) {
#if PROT_LEVEL > 0
    assert(inPath);
    assert(outPath);
#endif

    size_t bufSize;
    char *fileBuf = readFile(inPath, &bufSize);
    if (fileBuf == nullptr) {
        return ERR_FILE_RD;
    }
    printf("Buffer size read : %zu\n", bufSize);

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
    while (ip <= bufSize) {
        switch(codeBuf[ip]) {
            case CMD_HLT:
                writeCmd(outFile, CMD_HLT);
                ++ip;
                break;
#if PROT_LEVEL > 0 
            case CMD_VER:
                writeCmd(outFile, CMD_VER);
                ++ip;
                break;
            case CMD_DMP:
                writeCmd(outFile, CMD_DMP);
                ++ip;
                break;
#endif
            case CMD_OUT:
                writeCmd(outFile, CMD_OUT);
                ++ip;
                break;
            case CMD_PUSH:
                {
                    printf("Push command at %zu:\n", ip);
                    printf("Num starts at %zu and ends at %zu\n", ip + 1, ip + 1 + sizeof(num_t));
                    char *argStr = (char *)calloc(64, sizeof(*argStr));
                    sprintf(argStr, "%lld", *(num_t *)(codeBuf + ip + 1));
                    writeCmd(outFile, CMD_PUSH, argStr);
                    ip += 1 + sizeof(num_t);
                    free(argStr);
                    break;
                }
            case CMD_POP:
                writeCmd(outFile, CMD_POP);
                ++ip;
                break;
            case CMD_ADD:
                writeCmd(outFile, CMD_ADD);
                ++ip;
                break;
            case CMD_SUB:
                writeCmd(outFile, CMD_SUB);
                ++ip;
                break;
            case CMD_MUL:
                writeCmd(outFile, CMD_MUL);
                ++ip;
                break;
            case CMD_DIV:
                writeCmd(outFile, CMD_DIV);
                ++ip;
                break;
            default:
                printf("Undefined command\n");
                free(fileBuf);
                fclose(outFile);
                return ERR_UNDEF_CMD;
        }
    }

    free(fileBuf);
    if (fclose(outFile) == EOF) {
        return ERR_FILE_CLS;
    }

    return 0;
}
