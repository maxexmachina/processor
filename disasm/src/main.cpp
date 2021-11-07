#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "decompiler.h"
#include "../../include/fileUtils.h"

int main(int argc, char **argv) {
    if (argc == 2) {
        int ret = decompile(argv[1], "decompiled.asm");
        if (ret != 0) {
            return ret;
        }
    } else if (argc == 4) {
        if (strcmp(argv[2], "-o") != 0) {
            printf("Unexpected flag : %s\n", argv[2]);
            return EXIT_FAILURE;
        }
        int ret = decompile(argv[1], argv[3]);
        if (ret != 0) {
            return ret;
        }
    } else {
        printf("Unexpected number of arguments\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}
