#ifndef KERNEL_MEMORY_SYSTEM_MAP_HPP
#define KERNEL_MEMORY_SYSTEM_MAP_HPP

#include "loader/memory_descriptor.hpp"
#include <cstddef>
#include <stdint.h>

namespace memory {
    enum SysMemTypes { Free, Kernel };
    struct SystemMemMapEntry {
        SysMemTypes type;
        uint64_t start;
        size_t len;
    };

    extern SystemMemMapEntry memMap[0x400];
    extern uint64_t memMapLen;

    void initSystemMap(MemoryDescriptor* mDescs, size_t mDescsCount);
}

#endif // !KERNEL_MEMORY_SYSTEM_MAP_HPP
