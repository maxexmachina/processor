#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/fileUtils.h"
#include "../include/split.h"
#include "../include/stack.h"
#include "../include/processor.h"

int main(int argc, char **argv) {
    CmdBitMap bitmap;
    printf("%x\n", bitmap.ret);
/*    if (argc != 2) {
        printf("Please specify the program file path only\n");
        return EXIT_FAILURE;
    }
    const char *programPath = argv[1];
    Processor proc = {};

    int ret = ProcessorInit(&proc, programPath);
    if (ret != 0) {
        return ret;
    }

    ret = ProcessorRun(&proc);
    if (ret != 0) {
        return ret;
    }

    return EXIT_SUCCESS; 
    */
}
