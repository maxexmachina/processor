#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

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
int msleep(long tms) {
    struct timespec ts;
    int ret;

    if (tms < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;

    do {
        ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);

    return ret;
}

const char *getCmdName(int cmd) {
    for (size_t i = 0; i < CMD_SET_LEN; ++i) {
        if (CMD_NAME_MAP[i].id == cmd) {
            return CMD_NAME_MAP[i].name;
        }
    }
    return nullptr;
}

bool isEqualDouble(double lhs, double rhs) {
    return abs(lhs - rhs) < EPSILON;
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

	FILE *p = popen("tput cols && tput lines", "r");

	if(!p) {
        fprintf(stderr, "Error opening pipe.\n");
        return 1;
    }

	fscanf(p, "%zu\n%zu\n", &proc->term_width, &proc->term_height);

	if (pclose(p) == EOF) {
        fprintf(stderr," Error closing pipe!\n");
        return 1;
    }

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
			printf("Mul %ld and %ld\n", lhs, rhs);
            res = (double)lhs * (double)rhs / FX_POINT_PRECISION; 
			printf("Res : %ld\n", res);
            break;
        case OP_DIV:
            if (isEqualDouble(0, (double)(rhs / FX_POINT_PRECISION))) {
                fprintf(stderr, "Cannot divide by zero\n");
                return ERR_DIV_ZERO;
            }
			printf("div %ld and %ld\n", lhs, rhs);
            res = (double)lhs / (double)rhs * FX_POINT_PRECISION;
			printf("Res : %ld\n", res);
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
				if (!(type & RAM_MASK)) { 
					arg = proc->code[proc->ip++];
				}
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

double compareTopVals(Stack *stack, int *errCode) {
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
	printf("Comparing %ld and %ld \n", lhs, rhs);
	printf("Result: %f\n", (double)lhs / FX_POINT_PRECISION - (double)rhs / FX_POINT_PRECISION);
	if (isEqualDouble((double)lhs / FX_POINT_PRECISION,
		   	(double)rhs / FX_POINT_PRECISION)) {
		printf("Zero\n");
		return 0;
	}
	return (double)lhs / FX_POINT_PRECISION - (double)rhs / FX_POINT_PRECISION;
}

int ProcessorRun(Processor *proc) {
    assert(proc);
    assert(proc->code);

	//system("play -q sample.mp3");

    while (proc->ip < proc->codeSize) {
        int cmd = proc->code[proc->ip] & CMD_MASK;
        int type = proc->code[proc->ip];
        ++proc->ip;

		//printf("Command %s\n", getCmdName(cmd));
		//printf("Ip: %zu\n", proc->ip - 1);
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
                        printf("%.3f\n", (double)topElem / FX_POINT_PRECISION);
                    } else if (err == STK_UNDERFL) {
                        fprintf(stderr, "No elements on the stack\n");
						freeCpu(proc);
						return ERR_STK_POP;
                    } else {
                        fprintf(stderr, "Undefined error in StackPop\n");
						freeCpu(proc);
                        return ERR_STK_POP;
                    }
                    break;
                }
			case CMD_IN:
				{
					double num = 0;
					if (scanf("%lf", &num) != 1) {
						fprintf(stderr, "Error scanning the number\n");
						freeCpu(proc);
						return ERR_SCANF; 
					}
					num_t trunc_num = (num_t)(num * FX_POINT_PRECISION);
					int err = 0;
					StackPush(&proc->stack, &trunc_num, &err); 
					if (err) {
						return ERR_STK_PUSH;
					}
				}
				break;
            case CMD_PUSH:
                {
                    num_t arg = getArg(proc, cmd, type) / FX_POINT_PRECISION;
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
                        StackPop(&proc->stack, &proc->ram[arg / FX_POINT_PRECISION], &err);
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
			case CMD_ABS:
				{
					num_t num = 0;
					int err = 0;
					StackPop(&proc->stack, &num, &err);
					if (err) {
						fprintf(stderr, "Error in StackPop\n");
						freeCpu(proc);
						return ERR_STK_POP;
					}
					num = abs(num);
					StackPush(&proc->stack, &num, &err); 
					if (err) {
						fprintf(stderr, "Error in StackPush\n");
						freeCpu(proc);
						return ERR_STK_PUSH;
					}
					break;
				}
			case CMD_SQRT:
				{
					num_t num = 0;
					int err = 0;
					StackPop(&proc->stack, &num, &err);
					if (err) {
						fprintf(stderr, "Error in StackPop\n");
						freeCpu(proc);
						return ERR_STK_POP;
					}
					double num_sqrt = sqrt((double)num / FX_POINT_PRECISION);
					num = num_sqrt * FX_POINT_PRECISION;
					StackPush(&proc->stack, &num, &err); 
					if (err) {
						fprintf(stderr, "Error in StackPush\n");
						freeCpu(proc);
						return ERR_STK_PUSH;
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
				proc->ip = *(size_t *)(proc->code + proc->ip);
				break;
			case CMD_JA:
				if (compareTopVals(&proc->stack) > 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JAE:
				if (compareTopVals(&proc->stack) >= 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JB:
				if (compareTopVals(&proc->stack) < 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JBE:
				if (compareTopVals(&proc->stack) <= 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JE:
				if (compareTopVals(&proc->stack) == 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
				} else {
					proc->ip += sizeof(size_t);
				}
				break;
			case CMD_JNE:
				if (compareTopVals(&proc->stack) != 0) {
                    proc->ip = *(size_t *)(proc->code + proc->ip);
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
					proc->ip = *(size_t *)(proc->code + proc->ip);
				}
				break;
			case CMD_DRAW:
				{
					size_t address = VRAM_ADDR;
					for (size_t i = 0; i < proc->term_height; i++) {
						write(1, proc->ram + address, proc->term_width); 
						write(1, "\n", 1);
						address += proc->term_width;
					}
					break;
				}
			case CMD_SLEEP:
				msleep(1000 / 30);
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
