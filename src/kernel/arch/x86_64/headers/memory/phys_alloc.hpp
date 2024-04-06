#ifndef KERNEL_MEMORY_PHYS_ALLOC_HPP
#define KERNEL_MEMORY_PHYS_ALLOC_HPP

#include <cstdint>
namespace memory {
void initPhysAllocator();

void markBlockAsUsedAt64m(uint64_t start, uint64_t size);

inline void markBlockAsUsed(uint64_t start, uint64_t size) {
    markBlockAsUsedAt64m(start, size);
}
uint64_t allocate2mPage();
void free2mPage(uint64_t addr);
uint64_t allocate4kPage();
void free4kPage(uint64_t addr);
}

#endif // !KERNEL_MEMORY_PHYS_ALLOC_HPP
