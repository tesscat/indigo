#ifndef KERNEL_MEMORY_PAGE_ALLOC_HPP
#define KERNEL_MEMORY_PAGE_ALLOC_HPP

#include <paging.hpp>
#include <stdint.h>

namespace memory {
    void initPageAllocator();

    // Attempts to identity-map the 4KiB of memory starting at round-down
    // physAddr
    void kernelMap4KiBBlock(
        uint64_t virtAddr,
        uint64_t physAddr,
        uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE
    );
    // Attempts to identity-map the 2MiB of memory starting at round-down
    // physAddr
    inline void kernelIdentityMap4KiBBlock(
        uint64_t physAddr,
        uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE
    ) {
        kernelMap4KiBBlock(physAddr, physAddr, flags);
    }
    // If physAddr is mapped in the kernel space, unmaps it as a 4KiB block
    void kernelUnmap4KiBBlock(uint64_t virtAddr);
    // Attempts to identity-map the 2MiB of memory starting at round-down
    // physAddr
    void kernelMap2MiBBlock(
        uint64_t virtAddr,
        uint64_t physAddr,
        uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_BIG_PAGE
    );
    // Attempts to identity-map the 2MiB of memory starting at round-down
    // physAddr
    inline void kernelIdentityMap2MiBBlock(
        uint64_t physAddr,
        uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_BIG_PAGE
    ) {
        kernelMap2MiBBlock(physAddr, physAddr, flags);
    }
    // If physAddr is mapped in the kernel space, unmaps it as a 2MiB block
    void kernelUnmap2MiBBlock(uint64_t virtAddr);

    // identity-maps a block
    void kernelIdentityMapBlock(
        uint64_t base,
        uint64_t size,
        uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE
    );
    void kernelIdentityUnmapBlock(uint64_t base, uint64_t size);

    extern PageMapL4* kpgtable;

}

#endif // !KERNEL_MEMORY_PAGE_ALLOC_HPP
