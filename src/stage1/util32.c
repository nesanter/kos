#include "util32.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len++]);
    return len;
}

void memcpy(void *__dest, const void *__src, size_t __n) {
    while (--__n) {
        *(uint8_t*)__dest++ = *(uint8_t*)__src++;
    }
}

uint32_t strcmp(const char *__s1, const char *__s2) {
    while (*__s1 && *__s2 && *__s1++ == *__s2++);
    return (!*__s1 && !*__s2);
}

uint32_t strcmp_fix(const uint32_t *s1, const uint32_t *s2, uint32_t len) {
    while (--len) {
        if (*s1++ != *s2++)
            return 0;
    }
    return 1;
}
