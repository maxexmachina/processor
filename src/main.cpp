#include <stdio.h>
#include <stdlib.h>
#include <string.h> #include <errno.h>

#include "../include/fileUtils.h"
#include "../include/split.h"
#include "../include/compare.h"
#include "../include/stack.h"

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
        const char *command = "";
        long long int number = 0;
        int ret = sscanf(text.lines[i].ptr, "%ms %lld", &command, &number);
        if (mystrcmp(command, "push") == 0) {
            if (ret != 2) {
                printf("expected a number to push\n");
                return EXIT_FAILURE;
            }
            printf("number : %lld\n", number);
            StackPush(&stack, &number);
            long long int numCheck = 0;
            StackTop(&stack, &numCheck);
            printf("after push : %lld\n", numCheck);
            free((char *)command);
        }
    }
    printf("num lines: %zu\n", text.numLines);

    StackDtor(&stack);
    freeText(&text);
    return EXIT_SUCCESS; 
}
