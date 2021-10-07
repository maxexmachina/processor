#include <wchar.h>
#include <stdlib.h>

#include "../include/compare.h"

//TODO Toupper tolower
int mystrcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}
