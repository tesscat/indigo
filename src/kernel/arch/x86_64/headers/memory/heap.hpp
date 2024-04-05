#ifndef KERNEL_MEMORY_HEAP_HPP
#define KERNEL_MEMORY_HEAP_HPP

#include <stdint.h>

namespace memory {
void initHeap();
}

void* kmalloc(const uint64_t size);
void kfree(const void* addr);

#endif // !KERNEL_MEMORY_HEAP_HPP
