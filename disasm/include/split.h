#ifndef SPLIT_H
#define SPLIT_H

#include <sys/types.h>

//! Defines the default size of the array of lines that are read from the file
const size_t DEFAULT_ARR_SIZE = 1024;

//! Struct that holds information about the lines
typedef struct textLine {
    char *ptr;      /**< Pointer to the beginning of the line */ 
    size_t len;     /**< Pointer to the beginning of the line */
} line;

//------------------------------------------------------------ 
//! Splits the given char buffer into lines of text
//!
//! @param[in]  buffer      Char buffer to separate into lines
//! @param[in]  bufSize     Size of the buffer in bytes
//! @param[out] totalLines  Pointer to a variable to store the total number of lines separated
//!
//! @return An array of line structs that store info about lines of text in case of success, nullptr otherwise 
//------------------------------------------------------------ 
line *splitBuffer(char *buffer, size_t bufSize, size_t *totalLines);

#endif
