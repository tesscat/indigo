#ifndef KERNEL_MEMORY_PAGE_ALLOC_HPP
#define KERNEL_MEMORY_PAGE_ALLOC_HPP

#include <stdint.h>

namespace memory {
void initPageAllocator();

// Attempts to identity-map the 4KiB of memory starting at round-down physAddr
void kernelMap4KiBBlock(uint64_t physAddr);
// Attempts to identity-map the 2MiB of memory starting at round-down physAddr
void kernelMap2MiBBlock(uint64_t physAddr);

}

#endif // !KERNEL_MEMORY_PAGE_ALLOC_HPP
