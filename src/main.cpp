#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/fileUtils.h"
#include "../include/split.h"
#include "../include/compare.h"
#include "../include/stack.h"

struct command {
    const char text[16];
    long long int num;
};

int processCommand(Stack *stack, command *cmd, int ret) {
    if (mystrcmp(cmd->text, "push") == 0) {
        if (ret != 2) {
            printf("expected a number to push\n");
            return 0;
        }
        StackPush(stack, &cmd->num);
        StackDump(stack, "push");
    } else if (mystrcmp(cmd->text, "pop") == 0) {
        if (ret != 1) {
            printf("only expected a pop word\n");
            return 0;
        }
        long long temp = 0;
        StackPop(stack, &temp);
        StackDump(stack, "pop");
    } else if (mystrcmp(cmd->text, "dmp") == 0) {
        if (ret != 1) {
            printf("only expected a dmp word\n");
            return 0;
        }
        StackDump(stack, "Dump command");
    } else if (mystrcmp(cmd->text, "ver") == 0) {
        if (ret != 1) {
            printf("only expected a ver word\n");
            return 0;
        }
        int ret = StackError(stack);
        if (ret != 0) {
            printf("Stack verification failed: %d\n", ret);
            StackDump(stack, "Verificator failed");
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Please specify the program file path only\n");
        return EXIT_FAILURE;
    }
    const char *programPath = argv[1];

    text_t text = {};
    if (getText(programPath, &text) == 0) {
        printf("Error getting lines from file %s\n", programPath);
        return EXIT_FAILURE;
    }

    Stack stack = {};
    StackCtor(&stack, sizeof(elem_t), 10);

    for (size_t i = 0; i < text.numLines; ++i) {

        command cur = {};
        int ret = sscanf(text.lines[i].ptr, "%s %lld", &cur.text, &cur.num);
        if (ret == 0) {
            printf("Failed to scan a command : %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if (processCommand(&stack, &cur, ret) == 0) {
            return EXIT_FAILURE;
        }
    }
    printf("num lines: %zu\n", text.numLines);

    StackDtor(&stack);
    freeText(&text);
    return EXIT_SUCCESS; 
}
