#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP

#include "loader/memory_descriptor.hpp"

struct KernelArgs {
    unsigned int totalSize;
    unsigned int* framebuffer = nullptr;
    unsigned int fbWidth;
    unsigned int fbHeight;
    unsigned int fbPitch;
    int argc = 0;
    char** argv = nullptr;
    int memDescCount;
    MemoryDescriptor* memDesc;
    int kernelSegmentsCount;
    KernelSegmentLoc* kernelSegments;
};

#endif // !COMMON_LOADER_KERNEL_ARGS_HPP
