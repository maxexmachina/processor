#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../commands.h"
#include "../../config.h"
#include "processor.h"
#include "../../include/fileUtils.h"

void freeCpu(Processor *proc) {
    assert(proc);
    free(proc->code - TYPE_TAG_LEN - VER_LEN);
	free(proc->ram);
	StackDtor(&proc->stack);
	StackDtor(&proc->callStack);
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

    char tag[TYPE_TAG_LEN + 1] = "";
    memcpy(tag, fileBuf, TYPE_TAG_LEN);
    tag[TYPE_TAG_LEN] = '\0';
    printf("Read tag %s\n", tag);
    if (strcmp(tag, TYPE_TAG) != 0) {
        fprintf(stderr, "Binary file type tag %s doesn't match the required %s\n", tag, TYPE_TAG);
        free(fileBuf);
        return ERR_WRNG_TAG;
    }

    ver_t *ver = (ver_t *)(fileBuf + TYPE_TAG_LEN);
    printf("Read version %u\n", *ver);
    if (*ver != PROCESSOR_VER) {
        fprintf(stderr, "Specified command set version %u doesn't match the processor version %u\n", *ver, PROCESSOR_VER);
        free(fileBuf);
        return ERR_WRNG_CMD_SET;
    }
    proc->code = fileBuf + TYPE_TAG_LEN + VER_LEN; 
    proc->codeSize -= TYPE_TAG_LEN + VER_LEN;

	proc->ram = (char *)calloc(RAM_SIZE, sizeof(*proc->ram));
	if (!proc->ram) {
		free(fileBuf);
		return ERR_NOMEM;
	}

    StackCtor(&proc->stack, sizeof(num_t), DEFAULT_STACK_CAPACITY);
	StackCtor(&proc->callStack, sizeof(size_t), CALL_STACK_SIZE);
    proc->ip = 0;

    return 0;
}

int algebraicOperation(Stack *stack, AlgebraicOp op) {
    assert(stack);

    elem_t lhs = 0;
    elem_t rhs = 0;
	//TODO WHAT THE EFUCK
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
                fprintf(stderr, "Cannot divide by zero\n");
                return ERR_DIV_ZERO;
            }
            res = lhs / rhs;
            break;
        default:
            fprintf(stderr, "Undefined algebraic operation\n");
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
				if (type & RAM_MASK) arg = proc->ram[arg];
            }
            break;
        case CMD_POP:
			if (type & KONST_MASK) {
				if (type & RAM_MASK) {
					arg += *(num_t *)(proc->code + proc->ip);
					proc->ip += sizeof(num_t);
				}
			}
            if (type & REG_MASK) {
				if (!(type & RAM_MASK)) arg = proc->code[proc->ip++];
				else {
					arg += proc->regs[proc->code[proc->ip++] - 1];
				}
			}
            break;
        default:
            fprintf(stderr, "This command isn't supposted to have arguments\n");
    }
    return arg;
}

int compareTopVals(Stack *stack, int *errCode) {
    elem_t lhs = 0;
    elem_t rhs = 0;
    int err = 0;
    StackPop(stack, &rhs, &err);
    if (err != 0 && errCode) { 
		*errCode = err;
		return 0;
	}
    StackPop(stack, &lhs, &err);
    if (err != 0 && errCode) { 
		*errCode = err;
		return 0;
	}
	return lhs - rhs;
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
				freeCpu(proc);
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
                    StackPop(&proc->stack, &topElem, &err);
                    if (!err) { 
                        printf("%lld\n", topElem);
                    } else if (err == STK_UNDERFL) {
                        fprintf(stderr, "No elements on the stack\n");
                    } else {
                        fprintf(stderr, "Undefined error in StackPop\n");
						freeCpu(proc);
                        return ERR_STK_POP;
                    }
                    break;
                }
            case CMD_PUSH:
                {
                    num_t arg = getArg(proc, cmd, type);
                    int err = 0;
                    StackPush(&proc->stack, &arg, &err);
                    if (err) {
                        fprintf(stderr, "Error in StackPush\n");
						freeCpu(proc);
                        return ERR_STK_PUSH;
                    }
                    break;
                }
            case CMD_POP:
                {
                    num_t arg = getArg(proc, cmd, type);
                    int err = 0;
                    if (!(type & KONST_MASK) && !(type & REG_MASK) && !(type & RAM_MASK)) {
                        elem_t temp = 0;
                        StackPop(&proc->stack, &temp, &err); 
                    } else if ((type & REG_MASK) && !(type & RAM_MASK)) {
                        StackPop(&proc->stack, &proc->regs[arg - 1], &err);
                    } else if (type & RAM_MASK) {
                        StackPop(&proc->stack, &proc->ram[arg], &err);
					} else {
                        fprintf(stderr, "Wrong register number in pop command\n");
						freeCpu(proc);
                        return ERR_WRNG_REG; 
                    }
					if (err) {
						fprintf(stderr, "Error in StackPop\n");
						freeCpu(proc);
						return ERR_STK_POP;
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
			case CMD_JMP:
				proc->ip = proc->code[proc->ip++];
				break;
			case CMD_JA:
				if (compareTopVals(&proc->stack) > 0) {
					proc->ip = proc->code[proc->ip++];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JAE:
				if (compareTopVals(&proc->stack) >= 0) {
					proc->ip = proc->code[proc->ip];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JB:
				if (compareTopVals(&proc->stack) < 0) {
					proc->ip = proc->code[proc->ip];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JBE:
				if (compareTopVals(&proc->stack) <= 0) {
					proc->ip = proc->code[proc->ip];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JE:
				if (compareTopVals(&proc->stack) == 0) {
					proc->ip = proc->code[proc->ip];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JNE:
				if (compareTopVals(&proc->stack) != 0) {
					proc->ip = proc->code[proc->ip];
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
				/*
			case CMD_JF:
				proc->ip = proc->code[proc->ip];
				++proc->ip;
				break;
				*/
			case CMD_CALL:
				{
					size_t retAddr = proc->ip + sizeof(size_t);
					int err = 0;
					StackPush(&proc->callStack, &retAddr, &err);
                    if (err) {
                        fprintf(stderr, "Error in StackPush\n");
						freeCpu(proc);
                        return ERR_STK_PUSH;
                    }
					proc->ip = proc->code[proc->ip++];
				}
				break;
			case CMD_RET:
				{
					elem_t topElem = 0;
					int err = 0;
					StackPop(&proc->callStack, &topElem, &err);
					if (err == STK_UNDERFL) {
						fprintf(stderr, "No elements on the stack\n");
					} else if (err) {
						fprintf(stderr, "Undefined error in StackPop\n");
						freeCpu(proc);
						return ERR_STK_POP;
					}
					proc->ip = topElem;
				}
				break;
            default:
                fprintf(stderr, "Undefined command\n");
				freeCpu(proc);
                return ERR_UNDEF_CMD;
        }
    }

	freeCpu(proc);
    return 0;
}
