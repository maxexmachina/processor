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
            fprintf(stderr, "Failed to scan a command : %s\n", strerror(errno));
            break;
        case ERR_ARG_COUNT:
            fprintf(stderr, "Wrong number of command arguements\n");
            break;
        case ERR_UNDEF_CMD:
            fprintf(stderr, "Undefined command sequence\n");
            break;
        case ERR_CMD_BUFF_LEN:
            fprintf(stderr, "Too many commands for the set command buffer,"
                    "recompile with a bigger buffer size\n");
            break;
        case ERR_WRNG_ARG:
            fprintf(stderr, "Encountered unexpected argument\n");
            break;
        default:
            fprintf(stderr, "Undefined error occured\n");
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

int getCommand(const char *textLine, command_t *curCommand) {
    assert(textLine);
    assert(curCommand);

    if (sscanf(textLine, "%[^ ]s", curCommand->cmd) == 0) {
        return 1;
    } 
	printf("command : %s\n", curCommand->cmd);

    size_t numArgs = 0;
    curCommand->hasKonst = false;
    curCommand->hasReg = false;
	curCommand->hasRam = false;

	printf("%c\n", *(textLine + strlen(curCommand->cmd)));
	if (*(textLine + strlen(curCommand->cmd)) == 0) {
		numArgs = 0;
		return 0;
	}
	char *arg = (char *)(textLine + strlen(curCommand->cmd) + 1);

	num_t num = 0;
	char regChar = 0;

	printf("arg : %s\n", arg);
	if (sscanf(arg, "[%cx + %lld]", &regChar, &num) == 2) {
		curCommand->hasKonst = true;
		curCommand->konst = num;
		curCommand->hasReg = true;
		curCommand->reg = getRegId(regChar);
		curCommand->hasRam = true;
		numArgs = 2;
	} else if (sscanf(arg, "[%lld]", &num) == 1) {
		curCommand->hasKonst = true;
		curCommand->konst = num;
		curCommand->hasRam = true;
		numArgs = 1;
	} else if (sscanf(arg, "[%cx]", &regChar) == 1) {
		curCommand->hasReg = true;
		curCommand->reg = getRegId(regChar);
		curCommand->hasRam = true;
		numArgs = 1;
	} else if (sscanf(arg, "%cx + %lld", &regChar, &num) == 2) {
		curCommand->hasKonst = true;
		curCommand->konst = num;
		curCommand->hasReg = true;
		curCommand->reg = getRegId(regChar);
		numArgs = 2;
	} else if (sscanf(arg, "%lld", &num) == 1) {
		curCommand->hasKonst = true;
		curCommand->konst = num;
		numArgs = 1;
	} else if (sscanf(arg, "%cx", &regChar) == 1) {
		curCommand->hasReg = true;
		curCommand->reg = getRegId(regChar);
		numArgs = 1;
	} else {
		printf("Invalid argument fomatting\n");
		return 1;
	}
	printf("reg: %c; num: %lld\n", regChar, num);
	printf("Num args: %zu\n", numArgs);
	printf("\n");
	
    curCommand->numArgs = numArgs;

    return 0;
}

int getRegId(char startChar) {
    for (size_t i = 0; i < N_REGS; ++i) {
        if (REG_MAP[i].name[0] == startChar) {
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
        fprintf(stderr, "Error getting lines from file %s\n", inPath);
        return EXIT_FAILURE;
    }

    FILE *outFile = fopen(outPath, "w");
    if (outFile == nullptr) {
        //TODO stderr
        fprintf(stderr, "Error opening file %s : %s\n", outPath, strerror(errno));
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
        fprintf(stderr, "Error writing to file %s : %s\n", outPath, strerror(errno));
        freeText(&text);
        fclose(outFile);
        return ERR_FILE_WRT;
    }

    char *commandArray = (char *)calloc(MAX_CMD_ARR_LEN, sizeof(*commandArray));
    if (commandArray == nullptr) {
        fprintf(stderr, "Error allocating memory for the command array : %s\n", strerror(errno));
        freeText(&text);
        fclose(outFile);
        return ERR_NOMEM;
    }

    size_t pc = 0;
    for (size_t i = 0; i < text.numLines; ++i) {
        command_t cur = {};
        if (getCommand(text.lines[i].ptr, &cur) != 0) {
            return printCompilationError(ERR_CMD_SCAN, i, inPath,
                    commandArray, outFile, &text);
        }

        if (pc >= MAX_CMD_ARR_LEN - 1 - sizeof(num_t)) {
            return printCompilationError(ERR_CMD_BUFF_LEN, i, inPath,
                    commandArray, outFile, &text); 
        }

        //TODO hash string and switch on them
        if (strcmp(cur.cmd, "hlt") == 0) {
            if (cur.numArgs != 0) {
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
            if (cur.numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_OUT;
        } else if (strcmp(cur.cmd, "push") == 0) {
            if (cur.numArgs < 1) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
			char commandInfo = CMD_PUSH;
			char args[1 + sizeof(num_t)];
			size_t argLen = 0;
			if (cur.hasRam) {
				commandInfo |= RAM_BIT;
			}
			if (cur.hasKonst) {
				commandInfo |= KONST_BIT;
				num_t *ptr = (num_t *)args;
				*ptr = cur.konst;
				argLen += sizeof(num_t);
			}
			if (cur.hasReg) {
				commandInfo |= REG_BIT;
				if (cur.numArgs == 1) {
					args[0] = (char)cur.reg;
				} else {
					args[sizeof(num_t)] = cur.reg;
				}
				++argLen;
			}
			commandArray[pc++] = commandInfo;

			memcpy(commandArray + pc, args, argLen); 
			pc += argLen;
        } else if (strcmp(cur.cmd, "pop") == 0) {
            char commandInfo = CMD_POP;
			char args[1 + sizeof(num_t)];
			size_t argLen = 0;
			if (cur.hasRam) {
				commandInfo |= RAM_BIT;
			}
			if (cur.hasKonst) {
				if (!cur.hasRam) {
                    return printCompilationError(ERR_WRNG_ARG, i, inPath,
                            commandArray, outFile, &text);
				}
				commandInfo |= KONST_BIT;
				num_t *ptr = (num_t *)args;
				*ptr = cur.konst;
				argLen += sizeof(num_t);
			}
			if (cur.hasReg) {
				commandInfo |= REG_BIT;
				if (cur.numArgs == 1) {
					args[0] = (char)cur.reg;
				} else {
					args[sizeof(num_t)] = cur.reg;
				}
				++argLen;
			}
			commandArray[pc++] = commandInfo;

			memcpy(commandArray + pc, args, argLen); 
			pc += argLen;
        } else if (strcmp(cur.cmd, "add") == 0) {
            if (cur.numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_ADD;
        } else if (strcmp(cur.cmd, "sub") == 0) {
            if (cur.numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_SUB;
        } else if (strcmp(cur.cmd, "mul") == 0) {
            if (cur.numArgs != 0) {
                return printCompilationError(ERR_ARG_COUNT, i, inPath,
                        commandArray, outFile, &text);
            }
            commandArray[pc++] = CMD_MUL;
        } else if (strcmp(cur.cmd, "div") == 0) {
            if (cur.numArgs != 0) {
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
        fprintf(stderr, "Error writing to file %s : %s\n", outPath, strerror(errno));
        return ERR_FILE_WRT;
    }

    free(commandArray);
    if (fclose(outFile) == EOF) {
        freeText(&text);
        fprintf(stderr, "Error closing file %s : %s\n", outPath, strerror(errno));
        return ERR_FILE_CLS;
    }
    freeText(&text);

    return 0;
}
