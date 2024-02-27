#ifndef KERNEL_MEMORY_SYSTEM_MAP_HPP
#define KERNEL_MEMORY_SYSTEM_MAP_HPP

#include "loader/memory_descriptor.hpp"
#include <cstddef>
#include <stdint.h>

namespace memory {
enum SysMemTypes {
    Free,
    Kernel
};
struct SystemMemMapEntry {
    SysMemTypes type;
    uint64_t start;
    size_t len;
};

class SystemMap {

    SystemMemMapEntry* memMap;
    size_t memMapLen;
public:
    SystemMap(MemoryDescriptor* mDescs, size_t mDescsCount);
};
}

#endif // !KERNEL_MEMORY_SYSTEM_MAP_HPP
