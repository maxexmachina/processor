#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

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
    if (fread(readBuf, sizeof(*readBuf), fileSize, fileToRead) != fileSize) {
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
	printf("Num lines: %zu\n", text.numLines);

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
	freeText(&text);
	fprintf(outFile, "draw\njmp next\n");
	fclose(outFile);

	return 0;
}


const size_t MAX_FRAMES = 100000;

char	*substr(char const *s, size_t start, size_t len)
{
	if (!s)
		return nullptr;

	size_t s_len = strlen(s);
	size_t	cpy_len = 0;
	if (start > s_len)
		cpy_len = 0;
	else if (start + len > s_len)
		cpy_len = s_len - start;
	else
		cpy_len = len;

	char *ret = (char *)calloc(cpy_len + 1, sizeof(*ret));
	if (!ret) {
		return nullptr;
	}

	size_t i = 0;
	while (i < cpy_len) {
		ret[i] = s[start + i];
		++i;
	}
	ret[i] = '\0';
	return ret;
}

int compStrs(const void *str1, const void *str2) {
	const char *s1 = *(const char**)str1; 
	const char *s2 = *(const char**)str2; 

	const size_t len1 = strlen(s1);
	const size_t len2 = strlen(s2);

	char *sub1 = substr(s1, 3, len1 - 4);
	char *sub2 = substr(s2, 3, len2 - 4);
 
	int ret = atoi(sub1) -  atoi(sub2);

	free(sub1);
	free(sub2);

	return ret; 
}

int gen_asm_vid(const char *vidPath, unsigned int term_width, unsigned int term_height) {
	const char *asciiPath = "ascii.txt";

	printf("Generating frames\n");
	char *command = (char *)calloc(128, sizeof(*command));
	sprintf(command, "rm -rf frames && mkdir frames && ffmpeg -i %s -r 24/1 frames/out%%03d.jpg\n", vidPath); 
	system(command);
	free(command);

	char **filePaths = (char **)calloc(MAX_FRAMES, sizeof(*filePaths));
	DIR *dir;
	struct dirent *ent;

	printf("Reading and sorting frame dir\n");
	size_t numFiles = 0;
	if ((dir = opendir("frames/")) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			filePaths[numFiles++] = strdup(ent->d_name);
		}
		closedir(dir);
	} else {
		perror("Error opening directory");
		return EXIT_FAILURE;
	}
	qsort(filePaths, numFiles, sizeof(*filePaths), compStrs); 

	printf("Generating ascii file\n");
	for (size_t i = 2; i < numFiles; i++) {
		command = (char *)calloc(128, sizeof(*command));

		char fullPath[50] = "frames/";
		strcat(fullPath, filePaths[i]);
		sprintf(command, "jp2a --width=%u --height=%u %s >> %s\n", term_width, term_height, fullPath, asciiPath); 
		system(command);
		free(command);
	}

	printf("Reading ascii file lines\n");
	text_t text = {};

    if (getText(asciiPath, &text) == 0) {
        fprintf(stderr, "Error getting lines from file %s\n", asciiPath);
        return 1;
    }
	printf("Num lines: %zu\n", text.numLines);

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

	printf("Generating asm\n");
	fprintf(outFile, "next:\n");
	for (size_t frame = 0; frame < numFiles - 2; frame++) {
		for (size_t i = 0; i < term_height; i++) {
			for (size_t j = 0; j < text.lines[i].len; j++) {
				/*
				if (frame > 0) {
					if (text.lines[frame * term_height + i].ptr[j] !=
							text.lines[(frame - 1) * term_height + i].ptr[j]) {
						fprintf(outFile, "push %d\npop [%zu]\n", text.lines[frame * term_height + i].ptr[j],
								VRAM_ADDR + i * term_width + j);
					}
				} else {
				*/
				fprintf(outFile, "push %d\npop [%zu]\n", text.lines[frame * term_height + i].ptr[j],
						VRAM_ADDR + i * term_width + j);
			}
		}
		fprintf(outFile, "draw\nsleep\n");
	}
	fprintf(outFile, "jmp next\n");
	fclose(outFile);

	for (size_t i = 0; i < numFiles; i++) {
		free(filePaths[i]);
	}
	free(filePaths);
	freeText(&text);

	return 0;
}

int main(int argc, char **argv) {
	FILE *p = popen("tput cols && tput lines", "r");

	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
		return 1;
	}

	unsigned int term_width = 0;
	unsigned int term_height = 0;
	fscanf(p, "%u\n%u\n", &term_width, &term_height);
	printf("width : %u, height: %u\n", term_width, term_height);
	pclose(p);

	if (argc < 2) {
		printf("Please specify an image to convert\n");
		return 1;
	} else if (argc == 2) {
		const char *imgPath = argv[2];

		if (gen_asm(imgPath, term_width, term_height) != 0) {
			return 1;
		}
	} else if (argc == 3) {
		if (strcmp(argv[1], "-i") == 0) {
			const char *imgPath = argv[2];

			if (gen_asm(imgPath, term_width, term_height) != 0) {
				return 1;
			}
		} else if (strcmp(argv[1], "-v") == 0) {
			const char *vidPath = argv[2];

			if (gen_asm_vid(vidPath, term_width, term_height) != 0) {
				return 1;
			}
		} else {
			fprintf(stderr, "Unexpected flag %s\n", argv[1]);
			return 1;
		}
	} else {
		fprintf(stderr, "Too many arguments\n");
		return 1;
	}
}
