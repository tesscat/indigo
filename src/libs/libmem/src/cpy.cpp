#include <stdint.h>

void memcpy(const void* dest, const void* src, uint64_t n) {
    // TODO: use word-alignment
    for (uint64_t i = 0; i < n; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}
