#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "compiler.h"
#include "../../include/split.h"
#include "../../commands.h"
#include "../../config.h"

int printCompilationError(int errCode, size_t lineNum, const char *filePath,
        void *commandArray, FILE *outFile, text_t *text) {
    assert(filePath);
    assert(commandArray);
    assert(outFile);
    assert(text);

    printf("Error compiling file %s on line %zu:\n", filePath, lineNum + 1);

    switch(errCode) {
        case ERR_CMD_SCAN:
            printf("Failed to scan a command : %s\n", strerror(errno));
            break;
        case ERR_ARG_COUNT:
            printf("Wrong number of command arguements\n");
            break;
        case ERR_UNDEF_CMD:
            printf("Undefined command sequence\n");
            break;
        case ERR_CMD_BUFF_LEN:
            printf("Too many commands for the set command buffer,"
                    "recompile with a bigger buffer size\n");
            break;
        case ERR_WRNG_ARG:
            printf("Encountered unexpected argument\n");
            break;
        default:
            printf("Undefined error occured\n");
    }
    if (commandArray) {
        free(commandArray);
    }
    if (outFile) {
        fclose(outFile);
    }
    if (text) {
        freeText(text);
    }
    return errCode;
}

int getCommand(const char *textLine, command *curCommand, size_t *nArgs) {
    assert(textLine);
    assert(curCommand);
    assert(nArgs);

    if (sscanf(textLine, "%[^ ]s", curCommand->cmd) == 0) {
        return 1;
    } 

    size_t numArgs = 0;
    curCommand->hasKonst = false;
    curCommand->hasReg = false;
    size_t ret = 0;
    if (strchr(textLine, ' ') == nullptr) {
        numArgs = 0;
    } else if (strchr(textLine, '+') != nullptr) {
        numArgs = 2;
        const char *found = strchr(textLine, ' ');
        if (found != nullptr) {
            ret += sscanf(found + 1, "%[^+]s", curCommand->reg); 
            curCommand->hasReg = true;
        }
        found = strchr(textLine, '+');
        if (found != nullptr) {
            ret += sscanf(found + 1, "%lld", &curCommand->konst);
            curCommand->hasKonst = true;
        }
    } else {
        numArgs = 1;
        const char *found = strchr(textLine, ' ');
        if (found != nullptr) {
            size_t regRet = sscanf(found + 1, "%[a-z]s", curCommand->reg); 
            if (regRet != 0) { 
                curCommand->hasReg = true;
            }
            ret += regRet;
        }
        if (found != nullptr) {
            size_t konstRet = sscanf(found + 1, "%lld", &curCommand->konst);
            if (konstRet != 0) {
                curCommand->hasKonst = true;
            }
            ret += konstRet;
        }
    }
    if (ret != numArgs) {
        return 1; 
    }
    *nArgs = numArgs;

    return 0;
}

int getRegId(const char *name) {
    assert(name);

    for (size_t i = 0; i < N_REGS; ++i) {
        if (strcmp(REG_MAP[i].name, name) == 0) {
            return REG_MAP[i].id;
        }
    }

    return 0;
}

int compile(const char *inPath, const char *outPath) {
    assert(inPath);
    assert(outPath);

    text_t text = {};
    if (getText(inPath, &text) == 0) {
        printf("Error getting lines from file %s\n", inPath);
        return EXIT_FAILURE;
    }

    FILE *outFile = fopen(outPath, "w");
    if (outFile == nullptr) {
        //TODO stderr
        printf("Error opening file %s : %s\n", outPath, strerror(errno));
        freeText(&text);
        return ERR_FILE_OPN;
    }

    //TODO global tag constant, sizeof
    char tag[4 + sizeof(unsigned int)] = "JEFF";
    unsigned int curVersion = CMD_SET_VERSION;
    unsigned int *ver = (unsigned int *)(tag + 4);
    *ver = curVersion;
    if (fwrite(tag, sizeof(*tag), 4 + sizeof(unsigned int),
                outFile) != 4 + sizeof(unsigned int)) {
        printf("Error writing to file %s : %s\n", outPath, strerror(errno));
        freeText(&text);
        fclose(outFile);
        return ERR_FILE_WRT;
    }

    char *commandArray = (char *)calloc(MAX_CMD_ARR_LEN, sizeof(*commandArray));
    if (commandArray == nullptr) {
        printf("Error allocating memory for the command array : %s\n", strerror(errno));
        freeText(&text);
        fclose(outFile);
        return ERR_NOMEM;
    }

    size_t pc = 0;
    for (size_t i = 0; i < text.numLines; ++i) {
        command cur = {};
        size_t numArgs = 0;
        if (getCommand(text.lines[i].ptr, &cur, &numArgs) != 0) {
            return printCompilationError(ERR_CMD_SCAN, i, inPath,
                    commandArray, outFile, &text);
        }

        if (pc >= MAX_CMD_ARR_LEN - 1 - sizeof(num_t)) {
            return printCompilationError(ERR_CMD_BUFF_LEN, i, inPath,
                    commandArray, outFile, &text); 
        }

        //TODO hash string and switch on them
        if (strcmp(cur.cmd, "hlt") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_HLT; 
#if PROT_LEVEL > 0
        } else if (strcmp(cur.cmd, "ver") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_VER; 
        } else if (strcmp(cur.cmd, "dmp") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_DMP;
#endif
        } else if (strcmp(cur.cmd, "out") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_OUT;
        } else if (strcmp(cur.cmd, "push") == 0) {
            if (numArgs < 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            char commandInfo = CMD_PUSH;
            char args[1 + sizeof(num_t)];
            size_t argLen = 0;
            if (cur.hasKonst) {
                commandInfo |= KONST_BIT;
                num_t *ptr = (num_t *)args;
                *ptr = cur.konst;
                argLen += sizeof(num_t);
            }
            if (cur.hasReg) {
                commandInfo |= REG_BIT;
                if (numArgs == 1) {
                    args[0] = (char)getRegId(cur.reg);
                } else {
                    args[sizeof(num_t)] = getRegId(cur.reg);
                }
                ++argLen;
            }
            commandArray[pc++] = commandInfo;

            memcpy(commandArray + pc, args, argLen); 
            pc += argLen;
        } else if (strcmp(cur.cmd, "pop") == 0) {
            if (numArgs > 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            } 
            char commandInfo = CMD_POP;
            if (numArgs == 1) {
                if (!cur.hasReg) {
                    return printCompilationError(ERR_WRNG_ARG, i, inPath,
                            commandArray, outFile, &text);
                }
                commandInfo |= REG_BIT;
                commandArray[pc++] = commandInfo;
                commandArray[pc++] = getRegId(cur.reg);
            } else {
                commandArray[pc++] = commandInfo;
            }
        } else if (strcmp(cur.cmd, "add") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_ADD;
        } else if (strcmp(cur.cmd, "sub") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_SUB;
        } else if (strcmp(cur.cmd, "mul") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_MUL;
        } else if (strcmp(cur.cmd, "div") == 0) {
            if (numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_DIV;
        } else {
            return printCompilationError(ERR_UNDEF_CMD, i, inPath,
                    commandArray, outFile, &text);
        } 
    }

    if (fwrite(commandArray, sizeof(*commandArray), pc, outFile) != pc) {
        free(commandArray);
        freeText(&text);
        printf("Error writing to file %s : %s\n", outPath, strerror(errno));
        return ERR_FILE_WRT;
    }

    free(commandArray);
    if (fclose(outFile) == EOF) {
        freeText(&text);
        printf("Error closing file %s : %s\n", outPath, strerror(errno));
        return ERR_FILE_CLS;
    }
    freeText(&text);

    return 0;
}
