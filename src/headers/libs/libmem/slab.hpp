#ifndef LIBS_LIBMEM_SLAB_HPP
#define LIBS_LIBMEM_SLAB_HPP

#include <stdint.h>

namespace libmem {
template<typename T>
class SlabAllocator {
    T* values;
    // round-up
    uint64_t* bitmaps;
    uint64_t elements;
    uint64_t bitmapsCount;
public:
    SlabAllocator() {}
    SlabAllocator(T* memory, uint64_t elements_) : elements{elements_}, values {memory} {
        // we need to put the bitmaps into space <size> too
        // we assume that someone else has done the math, and it all fits in
        // put bitmaps high up in case T requires alignment
        bitmaps = (uint64_t*)memory[elements];
        bitmapsCount = (elements + 63)/64;
        for (uint64_t i = 0; i < bitmapsCount; i++) {
            bitmaps[i] = 0;
        }
    }

    void Free(T* element) {
        // zero the bitmap
        uint64_t loc = ((uint64_t)element - (uint64_t)values)/sizeof(T);
        uint64_t idx = loc/64;
        uint64_t offs = loc & 63;
        bitmaps[idx] &= ~(1 << offs);
    }
    T* Alloc() {
        // find the first non-full element
        uint64_t idx = -1;
        bool found = false;
        for (uint64_t i = 0; i < bitmapsCount; i++) {
            // TODO: is ( a != full ) or ( ~a != 0) better?
            if (bitmaps[i] != 0xffffffffffffffff) {
                idx = i;
                found = true;
                break;
            }
        }
        if (!found) return nullptr;
        uint64_t elem = bitmaps[idx];
        uint64_t offs = __builtin_ffs(~elem) - 1;
        uint64_t loc = idx*64 + offs;
        if (loc >= elements) return nullptr;
        bitmaps[idx] |= (1 << offs);
        T* element = &values[idx*64 + offs];
        return element;
    }
};
}

#endif // !LIBS_LIBMEM_SLAB_HPP
