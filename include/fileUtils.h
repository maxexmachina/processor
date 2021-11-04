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
// ! Checks if the given character is a whitespace
// !
// ! @param[in] c   Character to check
// !
// ! @return    true if the character is a whitespace, 0 otherwise
//------------------------------------------------------------ 
bool isSpace(const char c);

//------------------------------------------------------------ 
//! Cleans the given text buffer from \\r symbols and whitespaces at beginnings and ends of lines
//! 
//! @param[in]  readBuf Text buffer to clean
//! @param[in]  bufSize Size of the buffer
//! @param[out] numCleanBytes   Pointer to a variable to store the number of clean bytes in the returned buffer
//!
//! @return Clean text buffer where the first numCleanBytes are clean  
//------------------------------------------------------------ 
char *cleanBuffer(const char *readBuf, const size_t bufSize, size_t *numCleanBytes);

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

//------------------------------------------------------------ 
// ! Cleans the given text file from \\r symbols and spaces at beginnings and ends of lines,
// ! saves the new version to a new file 
// ! 
// ! @param[in] filePath    Relative path to file to clean
// ! @param[in] cleanPath   Relative path to the new cleaned file
// ! 
// ! @return    1 in case of successfull operation, 0 otherwise
//------------------------------------------------------------ 
int cleanFile(const char *filePath, const char *cleanPath);

#endif
