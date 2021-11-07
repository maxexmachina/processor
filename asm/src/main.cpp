#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "../../include/fileUtils.h"

int main(int argc, char **argv) {
    if (argc == 2) {
		int ret = cleanFile(argv[1], "clean.asm"); 
        ret = compile("clean.asm", "compiled.jf");
        return ret;
    } else if (argc == 4) {
        if (strcmp(argv[2], "-o") != 0) {
            printf("Unexpected flag : %s\n", argv[2]);
            return EXIT_FAILURE;
        }
		int ret = cleanFile(argv[1], "clean.asm"); 
        ret = compile("clean.asm", argv[3]);
        return ret;
    } else {
        printf("Unexpected number of arguments\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}
