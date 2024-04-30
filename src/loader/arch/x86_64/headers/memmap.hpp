#ifndef LOADER_MEMMAP_HPP
#define LOADER_MEMMAP_HPP

#include <stdint.h>
#define _STDINT_H
#include "uefi.h"

struct MemMapInfo {
    uint64_t fullSize;
    uint64_t descriptorSize;
};

uint64_t getMemmapSize();
MemMapInfo getMemmap(uint8_t* output);

#endif // !LOADER_MEMMAP_HPP
