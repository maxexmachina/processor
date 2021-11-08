#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>

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

	fprintf(stderr, "Error compiling file %s on line %zu:\n", filePath, lineNum + 1);
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

num_t doubleToNum(double num) {
	return (num_t)(num * FX_POINT_PRECISION);
}

int getCommand(const char *textLine, command_t *curCommand) {
    assert(textLine);
    assert(curCommand);

    if (sscanf(textLine, "%[^ ]s", curCommand->cmd) == 0) {
        return 1;
    } 

    size_t numArgs = 0;
    curCommand->hasKonst = false;
    curCommand->hasReg = false;
	curCommand->hasRam = false;
	curCommand->hasLabel = false;

	if (*(textLine + strlen(curCommand->cmd)) == 0) {
		numArgs = 0;
		return 0;
	}
	char *arg = (char *)(textLine + strlen(curCommand->cmd) + 1);

	double num = 0;
	char regChar = 0;

	if (strcmp(curCommand->cmd, "jmp") == 0 || 
		strcmp(curCommand->cmd, "ja") == 0 ||
		strcmp(curCommand->cmd, "jae") == 0 ||
		strcmp(curCommand->cmd, "jb") == 0 ||
		strcmp(curCommand->cmd, "jbe") == 0 ||
		strcmp(curCommand->cmd, "je") == 0 ||
		strcmp(curCommand->cmd, "jne") == 0 ||
		strcmp(curCommand->cmd, "jf") == 0 ||
		strcmp(curCommand->cmd, "call") == 0) {
		if (sscanf(arg, "%s", &curCommand->label) == 1) {
			curCommand->hasLabel = true;
			numArgs = 1;
		}
	} else { 
		if (sscanf(arg, "[%cx + %lf]", &regChar, &num) == 2) {
			curCommand->hasKonst = true;
			curCommand->konst = doubleToNum(num);
			curCommand->hasReg = true;
			curCommand->reg = getRegId(regChar);
			curCommand->hasRam = true;
			numArgs = 2;
		} else if (sscanf(arg, "[%lf]", &num) == 1) {
			curCommand->hasKonst = true;
			curCommand->konst = doubleToNum(num);
			curCommand->hasRam = true;
			numArgs = 1;
		} else if (sscanf(arg, "[%cx]", &regChar) == 1) {
			curCommand->hasReg = true;
			curCommand->reg = getRegId(regChar);
			curCommand->hasRam = true;
			numArgs = 1;
		} else if (sscanf(arg, "%cx + %lf", &regChar, &num) == 2) {
			curCommand->hasKonst = true;
			curCommand->konst = doubleToNum(num);
			curCommand->hasReg = true;
			curCommand->reg = getRegId(regChar);
			numArgs = 2;
		} else if (sscanf(arg, "%lf", &num) == 1) {
			curCommand->hasKonst = true;
			curCommand->konst = doubleToNum(num);
			numArgs = 1;
		} else if (sscanf(arg, "%cx", &regChar) == 1) {
			curCommand->hasReg = true;
			curCommand->reg = getRegId(regChar);
			numArgs = 1;
		} else {
			printf("Invalid argument fomatting\n");
			return 1;
		}
	}
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

int containsLabel(label *labels, char *name) {
	for (size_t i = 0; i < NUM_LABELS; ++i) {
		if (strcmp(labels[i].name, name) == 0) {
			return 0;
		}
	}
	return 1;
}

size_t getLabelAddr(label *labels, char *name) {
	for (size_t i = 0; i < NUM_LABELS; ++i) {
		if (strcmp(labels[i].name, name) == 0) {
			return labels[i].addr;
		}
	}
	return SIZE_MAX;
}

int addLabel(label *labels, char *name, size_t addr) {
	for (size_t i = 0; i < NUM_LABELS; ++i) {
		if (strcmp(labels[i].name, name) == 0) {
			labels[i].addr = addr;
			return 0;
		}
	}

	size_t i = 0;
	for (; i < NUM_LABELS && strcmp(labels[i].name, "") != 0; ++i)
		;
	if (i == NUM_LABELS) {
		fprintf(stderr, "No more labels pls\n");
		return 1;
	}
	labels[i].addr = addr;
	strcpy(labels[i].name, name);
	return 0;
 }

void handleLabel(label *labels, char *name, char* commandArray, size_t *pc) {
	if (containsLabel(labels, name)) {
		addLabel(labels, name, SIZE_MAX); 
		size_t *labelAddr = (size_t *)(commandArray + *pc);
		*labelAddr = SIZE_MAX;
		*pc += sizeof(size_t);
	} else {
		size_t *labelAddr = (size_t *)(commandArray + *pc);
		*labelAddr = getLabelAddr(labels, name);
		*pc += sizeof(size_t);
	}
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
        fprintf(stderr, "Error opening file %s : %s\n", outPath, strerror(errno));
        freeText(&text);
        return ERR_FILE_OPN;
    }

    char tag[TYPE_TAG_LEN + VER_LEN] = "";
	strcpy(tag, TYPE_TAG);
    unsigned int curVersion = CMD_SET_VERSION;
    unsigned int *ver = (ver_t *)(tag + TYPE_TAG_LEN);
    *ver = curVersion;
    if (fwrite(tag, sizeof(*tag), TYPE_TAG_LEN + VER_LEN,
                outFile) != TYPE_TAG_LEN + VER_LEN) {
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

	label labels[NUM_LABELS] = {};
	size_t pc;
	for (size_t iter = 0; iter < 2; ++iter) {
		pc = 0;
		for (size_t i = 0; i < text.numLines; ++i) {
			char line[MAX_LINE_LEN];
			if (sscanf(text.lines[i].ptr, "%s\n", line) == 1) {
				size_t len = strlen(line);
				if (line[len - 1] == ':') {
					char labelName[MAX_LABEL_LEN] = "";
					strncpy(labelName, line, len - 1);
					addLabel(labels, labelName, pc);
					continue;
				}
			}	

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
				if (cur.numArgs != 0) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_VER; 
			} else if (strcmp(cur.cmd, "dmp") == 0) {
				if (cur.numArgs != 0) {
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
			} else if (strcmp(cur.cmd, "in") == 0) {
				if (cur.numArgs != 0) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_IN;
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
						args[sizeof(num_t)] = (char)cur.reg;
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
			} else if (strcmp(cur.cmd, "sqrt") == 0) {
				if (cur.numArgs != 0) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_SQRT;
			} else if (strcmp(cur.cmd, "abs") == 0) {
				if (cur.numArgs != 0) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_ABS;
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
			} else if (strcmp(cur.cmd, "jmp") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JMP;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "ja") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JA;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "jae") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JAE;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "jb") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JB;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "jbe") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JBE;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "je") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JE;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "jne") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JNE;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "jf") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_JF;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "call") == 0) {
				if (cur.numArgs != 1) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_CALL;
				handleLabel(labels, cur.label, commandArray, &pc);
			} else if (strcmp(cur.cmd, "ret") == 0) {
				if (cur.numArgs != 0) {
					return printCompilationError(ERR_ARG_COUNT, i, inPath,
							commandArray, outFile, &text);
				}
				commandArray[pc++] = CMD_RET;
			} else {
				return printCompilationError(ERR_UNDEF_CMD, i, inPath,
						commandArray, outFile, &text);
			} 
		}
	}
	for (size_t i = 0; i < NUM_LABELS; i++) {
		printf("%s : %zu\n", labels[i].name, labels[i].addr);
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
