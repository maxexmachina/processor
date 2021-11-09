#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

enum FileType : int {
    TYPE_BINARY = 0,
    TYPE_TEXT = 1,
};

size_t getFileSize(const char *filePath) {
	struct stat fileStats = {};
    //TODO Error handling
    stat(filePath, &fileStats);
    return fileStats.st_size;
}

char *readFile(const char *filePath, size_t *size, FileType type) {
    const size_t fileSize = getFileSize(filePath);

    FILE *fileToRead = fopen(filePath, "r");
    //TODO Error handling macros
    if (fileToRead == nullptr) {
        printf("There was an error opening file %s : %s\n", filePath, strerror(errno));
        return nullptr;
    }

    char *readBuf = nullptr;
    if (type == TYPE_BINARY) {
        readBuf = (char *)calloc(fileSize, sizeof(*readBuf));
    } else if (type == TYPE_TEXT) {
        readBuf = (char *)calloc(fileSize + 1, sizeof(*readBuf));
    }

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

    if (type == TYPE_BINARY) {
        *size = fileSize; 
    } else if (type == TYPE_TEXT) {
        readBuf[fileSize] = '\0';
        *size = fileSize + 1;
    }
    return readBuf;
}

const size_t DEFAULT_ARR_SIZE = 1024;

//! Struct that holds information about the lines
typedef struct textLine {
    char *ptr;      /**< Pointer to the beginning of the line */ 
    size_t len;     /**< Pointer to the beginning of the line */
} line;

line *splitBuffer(char *buffer, size_t bufSize, size_t *totalLines) {
    size_t arrSize = DEFAULT_ARR_SIZE;
    line *lineArray = (line *)calloc(arrSize, sizeof(*lineArray));
    if (lineArray == nullptr) {
        printf("There was an error allocating memory : %s\n", strerror(errno));
        return nullptr;
    }
    
    size_t lineNum = 0;
    char *start = buffer;
    while (true) {
        char *endLine = strchr(buffer, '\n');
        if (endLine != nullptr) {
            lineArray[lineNum] = {.ptr = buffer,
                                  .len = (size_t)(endLine - buffer)};
            *endLine = '\0';
            buffer = endLine + 1;
            ++lineNum;
        } else {
            lineArray[lineNum] = {.ptr = buffer,
                                  .len = bufSize - (size_t)(buffer - start) - 1};
            break;
        }
        if (lineNum == arrSize - 1) {
            arrSize *= 2;
            line *newArr = (line *)realloc(lineArray, sizeof(*lineArray) * arrSize);
            if (newArr == nullptr) {
                printf("There was an error allocating memory : %s\n", strerror(errno));
                free(lineArray);
                return nullptr;
            }
            lineArray = newArr;
        }
    }
    *totalLines = lineNum;
    return lineArray;
}

struct text_t {
    char *buffer; /**< Buffer that stores the text*/
    line *lines; /**< Array of line structs that store info about text lines*/
    size_t numBytes; /**< Total number of bytes of text*/
    size_t numLines; /**< Total number of lines of text*/
};

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

const size_t VRAM_ADDR = 10;

int gen_asm(const char *imgPath, unsigned int term_width, unsigned int term_height) {
	const char *asciiPath = "ascii.txt";

	char *command = (char *)calloc(128, sizeof(*command));
	sprintf(command, "jp2a --width=%u --height=%u %s > %s\n", term_width, term_height, imgPath, asciiPath); 
	system(command);
	free(command);

	text_t text = {};

    if (getText(asciiPath, &text) == 0) {
        fprintf(stderr, "Error getting lines from file %s\n", asciiPath);
        return 1;
    }

	command = (char *)calloc(128, sizeof(*command));
	sprintf(command, "rm -rf %s",  asciiPath); 
	system(command);
	free(command);

    FILE *outFile = fopen("ascii.gasm", "w");
    if (outFile == nullptr) {
        fprintf(stderr, "Error opening file %s : %s\n", "ascii.gasm", strerror(errno));
        freeText(&text);
        return 1;
    }

	fprintf(outFile, "next:\n");
	size_t address = VRAM_ADDR;
	for (size_t i = 0; i < text.numLines; i++) {
		for (size_t j = 0; j < text.lines[i].len; j++) {
			fprintf(outFile, "push %d\npop [%zu]\n", text.lines[i].ptr[j], address++);
		}
	}
	fprintf(outFile, "draw\njmp next\n");
	fclose(outFile);

	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Please specify an image to convert\n");
		return 1;
	}

	const char *imgPath = argv[1];

	FILE *p = popen("tput cols && tput lines", "r");

	if(!p) {
        fprintf(stderr, "Error opening pipe.\n");
        return 1;
    }

	unsigned int term_width = 0;
	unsigned int term_height = 0;
	fscanf(p, "%u\n%u\n", &term_width, &term_height);
	pclose(p);

	if (gen_asm(imgPath, term_width, term_height) != 0) {
		return 1;
	}
}
