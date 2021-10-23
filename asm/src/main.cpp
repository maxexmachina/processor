#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"

int main(int argc, char **argv) {
    if (argc == 2) {
        int ret = compile(argv[1], "compiled.jf");
        return ret;
    } else if (argc == 4) {
        if (strcmp(argv[2], "-c") != 0) {
            printf("Unexpected flag : %s\n", argv[2]);
            return EXIT_FAILURE;
        }
        int ret = compile(argv[1], argv[3]);
        return ret;
    } else {
        printf("Unexpected number of arguments\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}
