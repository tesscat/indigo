// This file is responsible for implementing the physical page allocator.
// This will probably make me cry

// We are going to use a buddy-style allocator, with (twobit)-maps
// such that 00 = completely free,
// 01 = partially free
// 11 = completely full
//
// 10 is undefined
//
// There can be desync!! We can mark a higher level page as Completely Taken and all the lower pages don't need updating.
// The highest level must always hold true.
// We use 4KiB, 128KiB, 2M, 64M, 128M sections

#include <cstdint>
#include <memory/system_map.hpp>
#include <util/util.hpp>
#include <entry.hpp>

extern "C" char _kernel_phys_end;
extern "C" char _kernel_phys_start;

#define N_MAPS 4

const uint64_t sectSizes[] = {4*KiB, 128*KiB, 2*MiB, 64*MiB/* , 128*MiB */};
uint64_t* maps[N_MAPS];
uint64_t mapSizes[N_MAPS];

#define idx_var(addr, sectSize) (addr/(32*((uint64_t)sectSize)))
#define offs_var(addr, sectSize) ((addr/sectSize) % 32)

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
#define used_64bits 0xaaaaaaaaaaaaaaaa // 0xa == 0b1010

namespace memory {

inline void or4kAddr(uint64_t addr, uint8_t val) {
    maps[0][idx_4k(addr)] |= (val << offs_4k(addr));
}
inline void or128kAddr(uint64_t addr, uint8_t val) {
    maps[1][idx_128k(addr)] |= (val << offs_128k(addr));
}
inline void or2mAddr(uint64_t addr, uint8_t val) {
    maps[2][idx_2m(addr)] |= (val << offs_2m(addr));
}
inline void or64mAddr(uint64_t addr, uint8_t val) {
    maps[2][idx_64m(addr)] |= (val << offs_64m(addr));
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
    maps[2][idx_64m(addr)] &= ~(uint64_t)(0b11 << offs_64m(addr));
}


inline void mark64mAsUsed(uint64_t addr) {
    or64mAddr(addr, 0b11);
}
void mark2mAsUsed(uint64_t addr) {
    or2mAddr(addr, 0b11);

    // check again
    // no mask this time
    if (!((maps[2][idx_2m(addr)] & used_64bits) == used_64bits)) {
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
    if (!((maps[1][idx_128k(addr)] & used_64bits & mask) == (used_64bits & mask))) {
        // mark higher as partial
        or2mAddr(addr, 0b01);
        or64mAddr(addr, 0b01);
        return;
    }
    // mark us used
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
// TODO NOW add this for the other sizes when I wake up

void initPhysAllocator() {
    uint64_t kernel_virt_end = 0xffffe00000000000 + (uint64_t) &_kernel_phys_end;
    // how big will the bitmaps need to be?
    uint64_t memSize = memory::memMap[memory::memMapLen - 1].start + memory::memMap[memory::memMapLen - 1].len;
    uint64_t mapSize = 0;
    for (uint64_t sect : sectSizes) {
        mapSize += util::ceilDiv(memSize, sect);
    }
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
    // okay what my kernel look like
    markBlockAsUsedAt4k((uint64_t)&_kernel_phys_start, (uint64_t)&_kernel_phys_end - (uint64_t)&_kernel_phys_start);
    // we also got these maps, and kargs
    markBlockAsUsedAt4k((uint64_t)kargs - 0xffffe00000000000, kargs->totalSize);
    markBlockAsUsedAt4k((uint64_t)maps[0], mapSize);

    // done!
}

void markBlockAsUsed() {

}

}
