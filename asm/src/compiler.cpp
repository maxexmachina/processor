#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/compiler.h"
#include "../include/split.h"
#include "../../commands.h"
#include "../../config.h"

int printCompilationError(int errCode, size_t lineNum, const char *filePath,
        void *commandArray, FILE *outFile, text_t *text) {
    printf("Error compiling file %s on line %zu:\n", filePath, lineNum);

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

int compile(const char *inPath, const char *outPath) {

    text_t text = {};
    if (getText(inPath, &text) == 0) {
        printf("Error getting lines from file %s\n", inPath);
        return EXIT_FAILURE;
    }

    FILE *outFile = fopen(outPath, "w");
    if (outFile == nullptr) {
        printf("Error opening file %s : %s\n", outPath, strerror(errno));
        freeText(&text);
        return ERR_FILE_OPN;
    }

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
        int ret = sscanf(text.lines[i].ptr, "%s %lld", &cur.text, &cur.num);

        if (ret == 0) {
            return printCompilationError(ERR_CMD_SCAN, i, inPath,
                    commandArray, outFile, &text);
        }

        if (pc >= MAX_CMD_ARR_LEN - 9) {
            return printCompilationError(ERR_CMD_BUFF_LEN, i, inPath,
                    commandArray, outFile, &text); 
        }

        if (strcmp(cur.text, "hlt") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_HLT; 
#if PROT_LEVEL > 0
        } else if (strcmp(cur.text, "ver") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_VER; 
        } else if (strcmp(cur.text, "dmp") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_DMP;
#endif
        } else if (strcmp(cur.text, "out") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_OUT;
        } else if (strcmp(cur.text, "push") == 0) {
            if (ret != 2) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_PUSH;
            num_t *cmdPtr = (num_t *)(commandArray + pc);
            *cmdPtr = cur.num;
            pc += sizeof(*cmdPtr);
        } else if (strcmp(cur.text, "pop") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_POP;
        } else if (strcmp(cur.text, "add") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_ADD;
        } else if (strcmp(cur.text, "sub") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_SUB;
        } else if (strcmp(cur.text, "mul") == 0) {
            if (ret != 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_MUL;
        } else if (strcmp(cur.text, "div") == 0) {
            if (ret != 1) {
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
