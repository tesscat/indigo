#ifndef LIBS_LIBMEM_SLAB_HPP
#define LIBS_LIBMEM_SLAB_HPP

#include <stdint.h>

namespace libmem {
template<typename T, uint64_t elements>
class SlabAllocator {
    T* values = nullptr;
    // round-up
    uint64_t bitmaps[(elements+63)/64];
public:
    SlabAllocator() {
        // unmark everything
        for (uint64_t i = 0; i < elements; i++) {
            bitmaps[i] = 0;
        }
    }
    void SetMemoryAddr(uint64_t values_) {
        values = (T*)values_;
    }
    void Free(T* element) {
        // destroy the element if we have a dtor
        element->~T();
        // zero the bitmap
        uint64_t loc = (element - values)/sizeof(T);
        uint64_t idx = loc/64;
        uint64_t offs = loc & 63;
        bitmaps[idx] &= ~(1 << offs);
    }
    T* Alloc() {
        // find the first non-full element
        uint64_t idx = -1;
        bool found = false;
        for (uint64_t i = 0; i < elements; i++) {
            // TODO: is ( a != full ) or ( ~a != 0) better?
            if (bitmaps[i] != 0xffffffffffffffff) {
                idx = i;
                found = true;
                break;
            }
        }
        if (!found) return nullptr;
        uint64_t elem = bitmaps[idx];
        uint64_t offs = __builtin_ffs(elem) - 1;
        uint64_t loc = idx*64 + offs;
        if (loc >= elements) return nullptr;
        bitmaps[idx] |= (1 << offs);
        T* element = values[idx*64 + offs];
        element->T();
        return element;
    }
};
}

#endif // !LIBS_LIBMEM_SLAB_HPP
