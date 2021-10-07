#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "../include/fileUtils.h"

size_t getFileSize(const char *filePath) {
    struct stat fileStats = {};
    //TODO Error handling
    stat(filePath, &fileStats);
    return fileStats.st_size;
}

char *readFile(const char *filePath, size_t *size) {
    const size_t fileSize = getFileSize(filePath);

    FILE *fileToRead = fopen(filePath, "r");
    //TODO Error handling macros
    if (fileToRead == nullptr) {
        printf("There was an error opening file %s : %s\n", filePath, strerror(errno));
        return nullptr;
    }
    char *readBuf = (char *)calloc(fileSize + 1, sizeof(*readBuf));
    if (readBuf == nullptr) {
        printf("There was an error allocating memory : %s\n", strerror(errno));
        fclose(fileToRead);
        return nullptr;
    }
    if (fread(readBuf, 1, fileSize, fileToRead) != fileSize) {
        printf("There was an error reading file %s : %s\n", filePath, strerror(errno));
        free(readBuf);
        fclose(fileToRead);
        return nullptr;
    } 
    if (fclose(fileToRead) == EOF) {
        printf("There was an error closing file %s : %s\n", filePath, strerror(errno));
        free(readBuf);
        return nullptr; 
    }
    readBuf[fileSize] = '\0';
    *size = fileSize + 1;
    return readBuf;
}
