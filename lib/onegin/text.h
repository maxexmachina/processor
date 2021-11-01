#ifndef TEXT_H
#define TEXT_H

#include "split.h"

//! Stores information about the text read from the file
struct text_t {
    char *buffer; /**< Buffer that stores the text*/
    line *lines; /**< Array of line structs that store info about text lines*/
    size_t numBytes; /**< Total number of bytes of text*/
    size_t numLines; /**< Total number of lines of text*/
};

//------------------------------------------------------------ 
//! Frees the dynamic memory allocated for text storage
//!
//! @param[in]  text    text_t struct that stores info about the text
//------------------------------------------------------------ 
void freeText(text_t *text);

//------------------------------------------------------------ 
//! Fills a text_t struct with text read from a file
//!
//! @param[in]  filePath    Relative path to file to read the text from
//! @param[out] text        Pointer to text_t struct in which to store text and info
//!
//! @return     1 in case of success, 0 otherwise
int getText(const char *filePath, text_t *text);

#endif
