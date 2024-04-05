#ifndef KERNEL_MEMORY_PAGE_ALLOC_HPP
#define KERNEL_MEMORY_PAGE_ALLOC_HPP

#include <stdint.h>

namespace memory {
void initPageAllocator();

// Attempts to identity-map the 4KiB of memory starting at round-down physAddr
void kernelMap4KiBBlock(uint64_t physAddr);
// If physAddr is mapped in the kernel space, unmaps it as a 4KiB block
void kernelUnmap4KiBBlock(uint64_t physAddr);
// Attempts to identity-map the 2MiB of memory starting at round-down physAddr
void kernelMap2MiBBlock(uint64_t physAddr);
// If physAddr is mapped in the kernel space, unmaps it as a 2MiB block
void kernelUnmap2MiBBlock(uint64_t physAddr);

void kernelMapBlock(uint64_t base, uint64_t size);
void kernelUnmapBlock(uint64_t base, uint64_t size);


}

#endif // !KERNEL_MEMORY_PAGE_ALLOC_HPP
