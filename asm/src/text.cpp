#include <stdlib.h>

#include "../include/text.h"
#include "../include/fileUtils.h"

void freeText(text_t *text) {
    free(text->lines);
    free(text->buffer);
}

int getText(const char *filePath, text_t *text) {
    size_t bufSize = 0;
    char *textBuffer = readFile(filePath, &bufSize, TYPE_TEXT);
    if (textBuffer == nullptr) {
        return 0;
    }

    size_t numLines = 0;
    line *lineArray = splitBuffer(textBuffer, bufSize, &numLines); 
    if (lineArray == nullptr) {
        free(textBuffer);
        return 0;
    }
    text->buffer = textBuffer;
    text->lines = lineArray;
    text->numBytes = bufSize;
    text->numLines = numLines;
    return 1; 
}
