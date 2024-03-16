#ifndef KERNEL_UTIL_BASIC_HEAP_HPP
#define KERNEL_UTIL_BASIC_HEAP_HPP

#include <cstddef>
#include <stdint.h>

namespace memory::basic_heap {
struct MemHeader;

class BasicHeap {
    void* start;
    size_t len;

    void TryMergeWithSuccessor(MemHeader* mh);
public:
    BasicHeap(void* start_addr, size_t len);

    void* Allocate(const size_t size);
    int Free(const void* addr);
};
}

#endif // !KERNEL_UTIL_BASIC_HEAP_HPP
