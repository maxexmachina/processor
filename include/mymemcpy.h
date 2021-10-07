#ifndef MYMEMCPY_H
#define MYMEMCPY_H

#include <sys/types.h>

//------------------------------------------------------------ 
//! Copies n bytes of memory area src to memory area dest 
//!
//! @param[in]  src     Pointer to source area
//! @param[out] dest    Pointer to destination area 
//! @param[in]  n       Number of bytes to copy
//------------------------------------------------------------ 
void *myMemCpy(void *dest, void *src, size_t n);

#endif
