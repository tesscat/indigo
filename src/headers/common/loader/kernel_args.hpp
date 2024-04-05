#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP

#include "loader/memory_descriptor.hpp"
#include <loader/acpi_partial.hpp>

struct KernelArgs {
    // size of kernelArgs + all the funky addons
    unsigned int totalSize;
    unsigned int n_cpus;
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
