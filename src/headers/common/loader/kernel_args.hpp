#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP

#include "loader/memory_descriptor.hpp"
#include <loader/acpi_partial.hpp>

struct KernelArgs {
    // size of kernelArgs + all the funky addons
    unsigned int totalSize;
    unsigned int n_cpus;
    // location of the first module
    uint8_t* fsDriver;
    // length of first module
    uint64_t fsDriverLen;
    // location of initrd
    uint8_t* initrd;
    // length of initrd
    uint64_t initrdLen;
    // location of kernel elf
    uint8_t* kElf;
    uint64_t kElfLen;
    // PHYSICAL addr of XSDT
    acpi::XSDT* xsdt;
    unsigned int* framebuffer;
    unsigned int fbWidth;
    unsigned int fbHeight;
    unsigned int fbPitch;
    bool fbIsBGR;
    int argc;
    char** argv;
    int memDescCount;
    MemoryDescriptor* memDesc;
    int kernelSegmentsCount;
    KernelSegmentLoc* kernelSegments;
};

#endif // !COMMON_LOADER_KERNEL_ARGS_HPP
