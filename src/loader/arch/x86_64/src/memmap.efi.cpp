#include "memmap.hpp"
#include "uefi.h"

uint64_t getMemmapSize() {
    // uint64_t mmapSize;
    unsigned long long mmapSize;
    BS->GetMemoryMap(&mmapSize, nullptr, nullptr, nullptr, nullptr);
    printf("%i", mmapSize);
    return mmapSize;
}

MemMapInfo getMemmap(uint8_t* output) {
    uint64_t size = getMemmapSize();
    uint64_t descriptorSize;
    BS->GetMemoryMap(&size, (efi_memory_descriptor_t*)output, nullptr, &descriptorSize, nullptr);
    printf("s%i", size);
    return MemMapInfo { .fullSize = size, .descriptorSize = descriptorSize};
}
