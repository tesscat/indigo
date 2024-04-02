// This file is responsible for implementing the physical page allocator.
// This will probably make me cry

// We are going to use a buddy-style allocator, with (twobit)-maps
// such that 00 = completely free,
// 01 = partially free
// 11 = completely full
//
// 10 is undefined
//
// This means that _1 <=> not worth searching for the current size and vice versa,
// and 0_ <=> worth searching for smaller sizes and vice versa.
//
// There can be desync!! We can mark a higher level page as Completely Taken and all the lower pages don't need updating.
// The highest level must always hold true.
// We use 4KiB, 128KiB, 2M, 64M, 128M sections

#include <cstdint>
#include <memory/system_map.hpp>
#include <memory/phys_alloc.hpp>
#include <util/util.hpp>
#include <entry.hpp>

extern "C" char _kernel_phys_end;
extern "C" char _kernel_phys_start;

#define N_MAPS 4

const uint64_t sectSizes[] = {4*KiB, 128*KiB, 2*MiB, 64*MiB/* , 128*MiB */};
uint64_t* maps[N_MAPS];
// the size in Individual Markers (32 per 64bits)
uint64_t mapSizes[N_MAPS];

#define idx_var(addr, sectSize) ((addr)/(32*((uint64_t)(sectSize))))
#define offs_var(addr, sectSize) ((((addr)/(sectSize)) % 32)*2)

#define idx_4k(addr) idx_var(addr, 4*KiB)
#define offs_4k(addr) offs_var(addr, 4*KiB)
#define idx_128k(addr) idx_var(addr, 128*KiB)
#define offs_128k(addr) offs_var(addr, 128*KiB)
#define ishigh_128k(addr) (((addr/(16*128*KiB)) % 2) == 1)
#define idx_2m(addr) idx_var(addr, 2*MiB)
#define offs_2m(addr) offs_var(addr, 2*MiB)
#define idx_64m(addr) idx_var(addr, 64*MiB)
#define offs_64m(addr) offs_var(addr, 64*MiB)

// how many of these go into the next level?
#define neighbour_size_4k (128/4)
#define neighbour_size_128k (2*KiB / 128)
#define neighbour_size_2m (64/2)
#define neighbour_size_64m (128/64)

#define max_64bits 0xffffffffffffffff
#define max_32bits (uint64_t)0xffffffff
// checks the first bit of each, since both 0b10 and 0b11 mean unusable
#define fully_used_64bits 0xaaaaaaaaaaaaaaaa // 0xa == 0b1010
#define partially_used_64bits 0x5555555555555555 // 0x5 = 0b0101

namespace memory {

// inline bitmap operations {{{
inline void or4kAddr(uint64_t addr, uint8_t val) {
    maps[0][idx_4k(addr)] |= (val << offs_4k(addr));
}
inline void or128kAddr(uint64_t addr, uint8_t val) {
    maps[1][idx_128k(addr)] |= (val << offs_128k(addr));
}
inline void or2mAddr(uint64_t addr, uint8_t val) {
    // uint64_t idx =
    auto idx = idx_2m(addr);
    auto offs = offs_2m(addr);
    maps[2][idx] |= (val << offs);
}
inline void or64mAddr(uint64_t addr, uint8_t val) {
    maps[3][idx_64m(addr)] |= (val << offs_64m(addr));
}

inline void clear4kAddr(uint64_t addr) {
    maps[0][idx_4k(addr)] &= ~(uint64_t)(0b11 << offs_4k(addr));
}
inline void clear128kAddr(uint64_t addr) {
    maps[1][idx_128k(addr)] &= ~(uint64_t)(0b11 << offs_128k(addr));
}
inline void clear2mAddr(uint64_t addr) {
    maps[2][idx_2m(addr)] &= ~(uint64_t)(0b11 << offs_2m(addr));
}
inline void clear64mAddr(uint64_t addr) {
    maps[3][idx_64m(addr)] &= ~(uint64_t)(0b11 << offs_64m(addr));
}
// }}}

// marking as used {{{
inline void mark64mAsUsed(uint64_t addr) {
    or64mAddr(addr, 0b11);
}
void mark2mAsUsed(uint64_t addr) {
    or2mAddr(addr, 0b11);

    // check again
    // no mask this time
    if (!((maps[2][idx_2m(addr)] & fully_used_64bits) == fully_used_64bits)) {
        or64mAddr(addr, 0b01);
        return;
    }
    // mark us used again
    mark64mAsUsed(addr);
}
void mark128kAsUsed(uint64_t addr) {
    // mark it as completely used
    or128kAddr(addr, 0b11);

    // ok do we need to escalate again?
    // we need to check 16 entries, or 32bits
    uint64_t mask = ishigh_128k(addr) ? (max_32bits << 32) : max_32bits;
    if (!((maps[1][idx_128k(addr)] & fully_used_64bits & mask) == (fully_used_64bits & mask))) {
        // mark higher as partial
        or2mAddr(addr, 0b01);
        or64mAddr(addr, 0b01);
        return;
    }
    // mark us used completely
    mark2mAsUsed(addr);
}
void mark4kAsUsed(uint64_t addr) {
    or4kAddr(addr, 0b11);
    // do the higher ones too
    // we need to check 128/4 or 32 entries, which is 64 bits :)
    // life is made easy since lowest level can only be 0b11 or 0b00
    if (!(maps[0][idx_4k(addr)] == max_64bits)) {
        // can't set as fully used, mark as partial
        // there is no situation where the higher level would be fully used
        // and we were marking as used, so we need not clear
        or128kAddr(addr, 0b01);
        or2mAddr(addr, 0b01);
        or64mAddr(addr, 0b01);
        return;
    }
    // mark it as completely used
    mark128kAsUsed(addr);
}
// }}}

// Clearing {{{
inline void free64m(uint64_t addr) {
    clear64mAddr(addr);
}
void free2m(uint64_t addr) {
    clear2mAddr(addr);

    // are all my siblings clear?
    if (!((maps[2][idx_2m(addr)]) == 0)) {
        // mark everything above as partially used
        // TODO: make specific unmarkFullUse so we don't need to do two BM operations
        clear64mAddr(addr);
        or64mAddr(addr, 0b01);
        return;
    }
    // clear above
    free64m(addr);
}
void free128k(uint64_t addr) {
    clear128kAddr(addr);
    // are all my siblings clear?
    // we need to mask :(
    uint64_t mask = ishigh_128k(addr) ? (max_32bits << 32) : max_32bits;
    if (!((maps[1][idx_128k(addr)] & mask) == 0)) {
        // mark everything above as partially used
        // TODO: make specific unmarkFullUse so we don't need to do two BM operations
        clear2mAddr(addr);
        or2mAddr(addr, 0b01);
        clear64mAddr(addr);
        or64mAddr(addr, 0b01);
        return;
    }
    free2m(addr);
}
void free4k(uint64_t addr) {
    clear4kAddr(addr);

    // are all my siblings clear?
    if (!((maps[0][idx_4k(addr)]) == 0)) {
        // mark everything above as partially used
        // TODO: make specific unmarkFullUse so we don't need to do two BM operations
        clear128kAddr(addr);
        or128kAddr(addr, 0b01);
        clear2mAddr(addr);
        or2mAddr(addr, 0b01);
        clear64mAddr(addr);
        or64mAddr(addr, 0b01);
        return;
    }
    // clear above
    free128k(addr);
}
// }}}

// marking blocks as used {{{
void markBlockAsUsedAt4k(uint64_t start, uint64_t size) {
    uint64_t pageAlignedAddr = start & ~((uint64_t)0b111111111111);
    for(; pageAlignedAddr < start + size; pageAlignedAddr += 4*KiB) {
        mark4kAsUsed(pageAlignedAddr);
    }
}
void markBlockAsUsedAt128k(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 128 * KiB)
        return markBlockAsUsedAt4k(start, size);
    // get the low-slice
    uint64_t low_128k_addr = util::roundUpToPowerOfTwo(start, 128*KiB);
    uint64_t low_size = low_128k_addr - start;
    if (low_size != 0)
        markBlockAsUsedAt4k(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 128*KiB <= (size - low_size); curr_size += 128*KiB) {
        mark128kAsUsed(curr_size + low_128k_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        markBlockAsUsedAt4k(low_128k_addr + curr_size, high_size);
    }
}
void markBlockAsUsedAt2m(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 2 * MiB)
        return markBlockAsUsedAt128k(start, size);
    // get the low-slice
    uint64_t low_2m_addr = util::roundUpToPowerOfTwo(start, 2*MiB);
    uint64_t low_size = low_2m_addr - start;
    if (low_size != 0)
        markBlockAsUsedAt128k(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 2*MiB <= (size - low_size); curr_size += 2*MiB) {
        mark2mAsUsed(curr_size + low_2m_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        markBlockAsUsedAt128k(low_2m_addr + curr_size, high_size);
    }
}
void markBlockAsUsedAt64m(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 64 * MiB)
        return markBlockAsUsedAt2m(start, size);
    // get the low-slice
    uint64_t low_64m_addr = util::roundUpToPowerOfTwo(start, 64*MiB);
    uint64_t low_size = low_64m_addr - start;
    if (low_size != 0)
        markBlockAsUsedAt2m(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 64*MiB <= (size - low_size); curr_size += 64*MiB) {
        mark64mAsUsed(curr_size + low_64m_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        markBlockAsUsedAt2m(low_64m_addr + curr_size, high_size);
    }
}

void markBlockAsUsed(uint64_t start, uint64_t size) {
    markBlockAsUsedAt64m(start, size);
}

// }}}

// clearing blocks {{{
void freeBlockAt4k(uint64_t start, uint64_t size) {
    uint64_t pageAlignedAddr = start & ~((uint64_t)0b111111111111);
    for(; pageAlignedAddr < start + size; pageAlignedAddr += 4*KiB) {
        free4k(pageAlignedAddr);
    }
}
void freeBlockAt128k(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 128 * KiB)
        return freeBlockAt4k(start, size);
    // get the low-slice
    uint64_t low_128k_addr = util::roundUpToPowerOfTwo(start, 128*KiB);
    uint64_t low_size = low_128k_addr - start;
    if (low_size != 0)
        freeBlockAt4k(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 128*KiB <= (size - low_size); curr_size += 128*KiB) {
        free128k(curr_size + low_128k_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        freeBlockAt4k(low_128k_addr + curr_size, high_size);
    }
}
void freeBlockAt2m(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 2 * MiB)
        return freeBlockAt128k(start, size);
    // get the low-slice
    uint64_t low_2m_addr = util::roundUpToPowerOfTwo(start, 2*MiB);
    uint64_t low_size = low_2m_addr - start;
    if (low_size != 0)
        freeBlockAt128k(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 2*MiB <= (size - low_size); curr_size += 2*MiB) {
        free2m(curr_size + low_2m_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        freeBlockAt128k(low_2m_addr + curr_size, high_size);
    }
}
void freeBlockAt64m(uint64_t start, uint64_t size) {
    // if it has too small a size, immediately delegate
    if (size < 64 * MiB)
        return freeBlockAt2m(start, size);
    // get the low-slice
    uint64_t low_64m_addr = util::roundUpToPowerOfTwo(start, 64*MiB);
    uint64_t low_size = low_64m_addr - start;
    if (low_size != 0)
        freeBlockAt2m(start, low_size);
    // mark out any blocks of our size
    uint64_t curr_size = 0;
    for (; curr_size + 64*MiB <= (size - low_size); curr_size += 64*MiB) {
        free64m(curr_size + low_64m_addr);
    }
    // mark out the remaining high-slice
    uint64_t high_size = size - (curr_size + low_size);
    if (high_size != 0) {
        freeBlockAt2m(low_64m_addr + curr_size, high_size);
    }
}
// }}}

// finding frees {{{

// returns the index of the first partially free 64m block
// or -1 if not found
uint64_t findFirstPartial64mIdx() {
    // we can check 32 at a time
    for (uint64_t idx = 0; idx < mapSizes[3]/32; idx++) {
        uint64_t currBatch = maps[3][idx];
        // we want a 0 in the high slot
        currBatch = ~currBatch;
        // we want a 1 in the high slot
        if ((currBatch & fully_used_64bits) != 0) {
            // hit!
            // where it at
            uint64_t shiftIdx = 0;
            while ((0b10 & currBatch) == 0) {
                shiftIdx += 1;
                currBatch = currBatch >> 2;
            }
            return shiftIdx + idx*32;
        }
    }
    return -1;
}
// within the N-th 64-m block, find the first partially free 2m
// returns the Full (from-start) index
uint64_t findFirstPartial2mIdxWithin64m(uint64_t idx_64m) {
    // there are 32 2ms within one 64m, but each 32 takes one uint64_t
    uint64_t currBatch = maps[2][idx_64m];
    // we want a 0 in the high slot
    currBatch = ~currBatch;
    // we want a 1 in the high slot
    if ((currBatch & fully_used_64bits) != 0) {
        // hit!
        // where it at
        uint64_t shiftIdx = 0;
        while ((0b10 & currBatch) == 0) {
            shiftIdx += 1;
            currBatch = currBatch >> 2;
        }
        return shiftIdx + idx_64m*32;
    }
    return -1;
}
// within the N-th 64-m block, find the first fully free 2m
// returns the Full (from-start) index
uint64_t findFirstFullyFree2mIdxWithin64m(uint64_t idx_64m) {
    // there are 32 2ms within one 64m, but each 32 takes one uint64_t
    uint64_t currBatch = maps[2][idx_64m];
    // we want a 0 in the low slot
    currBatch = ~currBatch;
    // we want a 1 in the low slot
    if ((currBatch & fully_used_64bits) != 0) {
        // hit!
        // where it at
        uint64_t shiftIdx = 0;
        while ((0b01 & currBatch) == 0) {
            shiftIdx += 1;
            currBatch = currBatch >> 2;
        }
        return shiftIdx + idx_64m*32;
    }
    return -1;
}
// within the N-th 2-m block, find the first partially free 128k
// returns the Full (from-start) index
uint64_t findFirstPartial128kIdxWithin2m(uint64_t idx_2m) {
    // there are 16 128ks within one 2m, but each 16 takes one half of a uint64_t
    uint64_t currBatch = maps[1][idx_2m/2];
    // handle downshifting
    if (idx_2m % 2 == 1) {
        // odd. shift it to low-half
        currBatch = currBatch >> 32;
    }
    // clip the top (mark as fully used)
    currBatch |= 0xffffffff00000000;
    // we want a 0 in the high slot
    currBatch = ~currBatch;
    // we want a 1 in the high slot
    if ((currBatch & fully_used_64bits) != 0) {
        // hit!
        // where it at
        uint64_t shiftIdx = 0;
        while ((0b10 & currBatch) == 0) {
            shiftIdx += 1;
            currBatch = currBatch >> 2;
        }
        return shiftIdx + idx_2m*16;
    }
    return -1;
}
// within the N-th 128k block, find the first fully free 4k
// returns the Full (from-start) index
uint64_t findFirstFullyFree4kIdxWithin128k(uint64_t idx_128k) {
    // there are 32 4ks within one 128k, but each 32 takes one uint64_t
    uint64_t currBatch = maps[0][idx_128k];
    // we want a 0 in the low slot
    currBatch = ~currBatch;
    // we want a 1 in the low slot
    if ((currBatch & fully_used_64bits) != 0) {
        // hit!
        // where it at
        uint64_t shiftIdx = 0;
        while ((0b01 & currBatch) == 0) {
            shiftIdx += 1;
            currBatch = currBatch >> 2;
        }
        return shiftIdx + idx_128k*32;
    }
    return -1;
}

// }}}

// actually allocating {{{

uint64_t allocate2mPage() {
    // find first partial 64m
    uint64_t idx_64m = findFirstPartial64mIdx();
    if (idx_64m == (uint64_t)-1) return -1;
    uint64_t idx_2m = findFirstFullyFree2mIdxWithin64m(idx_64m);
    if (idx_2m == (uint64_t)-1) return -1;
    // we have our page
    // mark it as Completely Used
    uint64_t addr = 2*MiB*idx_2m;
    mark2mAsUsed(addr);
    return 2*MiB*idx_2m;
}
void free2mPage(uint64_t addr) {
    free2m(addr);
}


uint64_t allocate4kPage() {
    // find first partial 64m
    uint64_t idx_64m = findFirstPartial64mIdx();
    if (idx_64m == (uint64_t)-1) return -1;
    uint64_t idx_2m = findFirstPartial2mIdxWithin64m(idx_64m);
    if (idx_2m == (uint64_t)-1) return -1;
    uint64_t idx_128k = findFirstPartial128kIdxWithin2m(idx_2m);
    if (idx_128k == (uint64_t)-1) return -1;
    uint64_t idx_4k = findFirstFullyFree4kIdxWithin128k(idx_128k);
    if (idx_4k == (uint64_t)-1) return -1;
    // mark as used
    uint64_t addr = 4*KiB*idx_4k;
    mark4kAsUsed(addr);
    return 4*KiB*idx_4k;
}
void free4kPage(uint64_t addr) {
    free4k(addr);
}

// }}}

// TODO: include the dirty low stuff too, we can use that if we want to
// TODO: but make sure to clean it + mark stuff out that we can't use
void initPhysAllocator() {
    uint64_t kernel_virt_end = 0xffffe00000000000 + (uint64_t) &_kernel_phys_end;
    // how big will the bitmaps need to be?
    uint64_t memSize = memory::memMap[memory::memMapLen - 1].start + memory::memMap[memory::memMapLen - 1].len;
    uint64_t mapSize = 0;
    for (uint64_t sect : sectSizes) {
        mapSize += util::ceilDiv(memSize, sect);
    }
    graphics::psf::print("Memory is: ");
    util::printAsHex(memSize);
    // mapSize is now one byte for every bitmap entry
    mapSize = util::ceilDiv(mapSize, 4);

    // the area immediately above the kernel in physical memory is mapped, so we can use it as a kind-of
    // scratch buffer
    // it's very useful, for instance, now
    // page align it too since why not
    uint64_t* map_4k = (uint64_t*)kernel_virt_end + (4096 - (kernel_virt_end%4096));
    uint64_t map_4k_bits = util::ceilDiv(memSize, 4*KiB);

    uint64_t* map_128k = map_4k + util::ceilDiv(map_4k_bits, 4);
    map_128k += 4096 - ((uint64_t)map_128k)%4096;
    uint64_t map_128k_bits = util::ceilDiv(memSize, 128 * KiB);

    uint64_t *map_2m = map_128k + util::ceilDiv(map_128k_bits, 4);
    map_2m += 4096 - ((uint64_t)map_2m)%4096;
    uint64_t map_2m_bits = util::ceilDiv(memSize, 2 * MiB);

    uint64_t *map_64m = map_2m + util::ceilDiv(map_2m_bits, 4);
    map_64m += 4096 - ((uint64_t)map_64m)%4096;
    uint64_t map_64m_bits = util::ceilDiv(memSize, 64 * MiB);


    maps[0] = map_4k;
    mapSizes[0] = map_4k_bits;
    maps[1] = map_128k;
    mapSizes[1] = map_128k_bits;
    maps[2] = map_2m;
    mapSizes[2] = map_2m_bits;
    maps[3] = map_64m;
    mapSizes[3] = map_64m_bits;
    // next up is to get variables for their length, zero them, then mark what's already in use (kernel + this map itself + page tables)
    // zero them
    for (int i = 0; i < N_MAPS; i++) {
        for (uint64_t j = 0; j < util::ceilDiv(mapSizes[i], 4); j++) {
            maps[i][j] = 0;
        }
    }

    // mark the lowest 4k as used, for various reasons:
    // - low memory is dodgy, QEMU doesn't like it either so we prevent us using it
    // - in case we need anything in the future that is Absolutely Guaranteed to be free
    // - so we can use 0 as "non-existent" marker in various things
    markBlockAsUsedAt4k(0, 4*KiB - 1);

    // okay what my kernel look like
    markBlockAsUsedAt4k((uint64_t)&_kernel_phys_start, (uint64_t)&_kernel_phys_end - (uint64_t)&_kernel_phys_start);
    // we also got these maps, and kargs
    markBlockAsUsedAt4k((uint64_t)kargs - 0xffffe00000000000, kargs->totalSize);
    markBlockAsUsedAt4k((uint64_t)&maps[0] - 0xffffe00000000000, mapSize);

    uint64_t last_high = 0;

    // also fill out the used stuff from the memmap itself
    for(uint64_t i = 0; i < memory::memMapLen; i++) {
        // mark from last to current if it's not accounted for
        if (memory::memMap[i].start != last_high) {
            markBlockAsUsed(last_high, memMap[i].start - last_high);
        }
        // mark current segment
        if (memMap[i].type == Kernel) {
            markBlockAsUsed(memMap[i].start, memMap[i].len);
        }
        last_high = memMap[i].start + memMap[i].len;
    }


    // done!
}

}
