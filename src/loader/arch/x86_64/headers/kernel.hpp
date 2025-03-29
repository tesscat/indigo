#ifndef LOADER_KERNEL_HPP
#define LOADER_KERNEL_HPP

#include "loader/memory_descriptor.hpp"
#include "vec_efi/vec_efi.hpp"
#include <loader/kernel_args.hpp>
#include <ion_efi/ion_efi.hpp>

class Kernel {
    void (* __attribute__((sysv_abi)) entry)(KernelArgs*, uint64_t);
    uint64_t kEntry;
    uint64_t kArgsAddr;
    uint8_t* fsD;
    uint64_t fsDLen;
    uint8_t* initrdData;
    uint64_t initrdLen;
    uint8_t* kElf;
    uint64_t kElfLen;
    indigo::Vec<KernelSegmentLoc> kSegs;
public:
    Kernel(const char* path, ion::RootNode* root, const char* trampPath, const char* fsDriverPath, const char* initrdPath);
    void Run(size_t argc, char** argv) __attribute__((noreturn));
};

#endif // !LOADER_KERNEL_HPP
