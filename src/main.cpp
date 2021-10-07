#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/fileUtils.h"
#include "../include/split.h"

int main() {
    const char *programPath = "program.txt";
    text_t text = {};
    if (getText(programPath, &text) == 0) {
        printf("Error getting lines from file %s\n", programPath);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < text.numLines; ++i) {
        printf("%s\n", text.lines[i].ptr);
    }
    printf("num lines: %zu\n", text.numLines);

    freeText(&text);
    return EXIT_SUCCESS; 
}
