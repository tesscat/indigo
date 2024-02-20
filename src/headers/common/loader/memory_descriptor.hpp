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


inline bool isUsableMemory(MemoryType type) {
    // https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html
    return (1 <= type && 4 >= type) || type == 7;
}

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
