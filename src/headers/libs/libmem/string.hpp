#ifndef LIBS_LIBMEM_STRING_HPP
#define LIBS_LIBMEM_STRING_HPP
#include <stdint.h>

namespace libmem {
inline bool strncmp(const char* a, const char* b, const uint64_t n) {
    for (uint64_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
inline bool strcmp(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    };
    return *a == *b;
}
inline uint64_t strlen(const char* a) {
    uint64_t n = 0;
    while (a[n]) {n++;};
    return n;
}
}
#endif // !LIBS_LIBMEM_STRING_HPP
