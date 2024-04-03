#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP

#include "loader/memory_descriptor.hpp"

struct KernelArgs {
    // size of kernelArgs + all the funky addons
    unsigned int totalSize;
    unsigned int n_cpus;
    // PHYSICAL addr of XSDT
    void* xsdt;
    unsigned int* framebuffer = nullptr;
    unsigned int fbWidth;
    unsigned int fbHeight;
    unsigned int fbPitch;
    bool fbIsBGR;
    int argc = 0;
    char** argv = nullptr;
    int memDescCount;
    MemoryDescriptor* memDesc;
    int kernelSegmentsCount;
    KernelSegmentLoc* kernelSegments;
};

#endif // !COMMON_LOADER_KERNEL_ARGS_HPP
