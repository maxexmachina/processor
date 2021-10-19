#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <sys/types.h>

#include "split.h"
#include "text.h"

enum FileType : int {
    TYPE_BINARY = 0,
    TYPE_TEXT = 1,
};

//------------------------------------------------------------ 
//! Returns the size of a given file
//! 
//! @param[in]  filePath    Relative path to the file
//!
//! @return The amount of space the file takes up on the drive in bytes 
//------------------------------------------------------------ 
size_t getFileSize(const char *filePath);

//------------------------------------------------------------ 
//! Reads text from a file and returns a null-terminated buffer 
//!
//! @param[in]  filePath    Path to file
//! @param[out] size        Pointer to a variable to store the size of the buffer including the null-termination character
//!
//! @return     null-terminated char buffer in case of success, nullptr otherwise
//------------------------------------------------------------ 
char *readFile(const char *filePath, size_t *size, FileType type=TYPE_BINARY);

#endif
