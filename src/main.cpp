#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/fileUtils.h"
#include "../include/split.h"
#include "../include/stack.h"
#include "../include/processor.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Please specify the program file path only\n");
        return EXIT_FAILURE;
    }
    const char *programPath = argv[1];
    run(programPath);

    return EXIT_SUCCESS; 
}
