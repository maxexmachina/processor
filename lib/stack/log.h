#ifndef LOG_H
#define LOG_H

#include <stdio.h>

static FILE *logFile = nullptr;

enum LogErr : int {
    LOG_OPN_FAIL = -1,
    LOG_WRT_FAIL = 0,
};

//------------------------------------------------------------ 
//! Opens the log file and saves the file handle
//!
//! @return 1 on success, LOG_OPN_FAIL on failure
//------------------------------------------------------------ 
int initLog();

//------------------------------------------------------------ 
//! Writes a formatted string with parameters to the log file
//! 
//! @param[in]  format  Formatted string with parameters
//! @param[in]  ...     List of parameters to substitute in the string
//!
//! @return 1 on success, LOG_WRITE_FAIL on write failure, LOG_OPN_FAIL on log open failure
//------------------------------------------------------------ 
int writeToLog(const char *format ...);

//------------------------------------------------------------ 
//! Closes the log file
//!
//! @return 1 on success, 0 on failure
//------------------------------------------------------------ 
int closeLog();

#endif
