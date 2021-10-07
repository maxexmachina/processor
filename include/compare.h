#ifndef COMPARE_H
#define COMPARE_H

#include "../include/split.h"

//------------------------------------------------------------ 
//! Compares two given wide char strings alphabetically
//!
//! @param[in]  s1  First string to compare
//! @param[in]  s2  Second string to compare
//!
//! @return     Positive value if s1 > s2, 0 if s1 == s2, negative value if s1 < s2 
//------------------------------------------------------------ 
int mystrcmp(const char *s1, const char *s2);

#endif
