#include <stdarg.h>

#include "log.h"

int initLog() {
    logFile = fopen("stack_log.txt", "w");
    if (logFile == nullptr) {
        return LOG_OPN_FAIL;
    }
    return 1;
}

int writeToLog(const char *format ...) {
    if (logFile == nullptr) {
        if (initLog() == LOG_OPN_FAIL) {
            return LOG_OPN_FAIL;
        } 
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    int ret = vfprintf(logFile, format, arg_ptr);
    va_end(arg_ptr);
    if (ret < 0) {
        return LOG_WRT_FAIL;
    } else {
        return 1; 
    }
}

int closeLog() {
    if (logFile != nullptr) {
        if (fclose(logFile) == EOF) {
            return 0;
        } 
    }
    return 1;
}
