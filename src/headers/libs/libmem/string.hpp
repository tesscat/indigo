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
}
#endif // !LIBS_LIBMEM_STRING_HPP
