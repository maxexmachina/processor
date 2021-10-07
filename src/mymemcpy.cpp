#include "../include/mymemcpy.h"

void *myMemCpy(void *dest, void *src, size_t n) {
    char *cDest = (char *)dest;
    char *cSrc = (char *)src;
    while (true) {
        if (n >= 8) {
            long long *llDest = (long long *)cDest;
            *llDest = *(long long *)cSrc;
            cDest += 8;
            cSrc += 8;
            n -= 8;
        } else if (n >= 4) {
            int *iDest = (int *)cDest;
            *iDest = *(int *)cSrc;
            cDest += 4;
            cSrc += 4;
            n -= 4;
        } else if (n >= 2) {
            short *sDest = (short *)cDest;
            *sDest = *(short *)cSrc;
            cDest += 2;
            cSrc += 2;
            n -= 2;
        } else if (n >= 1) {
            *cDest = *cSrc;
            ++cDest;
            ++cSrc;
            --n;
        } else {
            break;
        }
    }
    return dest;
}
