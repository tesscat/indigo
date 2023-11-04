#include <stdint.h>

void memcpy(void* dest, void* src, uint64_t n) {
  // TODO: use word-alignment
  // make dest word-aligned
  // while (n > 0 && (uint64_t)dest % (sizeof(uint64_t)) != 0) {
  //   *(uint8_t*)dest = (uint8_t)*(uint8_t*)src;
  //   dest++;
  //   src++;
  // }
  for (uint64_t i = 0; i < n; i++) {
    ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
  }
  // while(n) {
  //   *(uint8_t*)dest = *(uint8_t*)src;
  //   n--;
  // }
}
