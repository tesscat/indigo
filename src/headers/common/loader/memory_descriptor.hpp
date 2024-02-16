#ifndef COMMON_LOADER_MEMORY_DESCRIPTOR_HPP
#define COMMON_LOADER_MEMORY_DESCRIPTOR_HPP

#include <stdint.h>

typedef enum {
    ReservedMemoryType,
    LoaderCode,
    LoaderData,
    BootServicesCode,
    BootServicesData,
    RuntimeServicesCode,
    RuntimeServicesData,
    ConventionalMemory,
    UnusableMemory,
    ACPIReclaimMemory,
    ACPIMemoryNVS,
    MemoryMappedIO,
    MemoryMappedIOPortSpace,
    PalCode,
    PersistentMemory,
    UnacceptedMemoryType,
    MaxMemoryType
} MemoryType;

struct MemoryDescriptor {
    MemoryType type : 32;
    uint64_t physStart;
    uint64_t nrPages;
    uint64_t attr;

    bool operator<(MemoryDescriptor other) {
        return physStart < other.physStart;
    }
};

struct KernelSegmentLoc {
    uint64_t physLoc;
    uint64_t len;
};

#endif // !COMMON_LOADER_MEMORY_DESCRIPTOR_HPP
