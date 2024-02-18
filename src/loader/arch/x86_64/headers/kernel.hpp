#ifndef LOADER_KERNEL_HPP
#define LOADER_KERNEL_HPP

#include "loader/memory_descriptor.hpp"
#include "vec_efi/vec_efi.hpp"
#include <loader/kernel_args.hpp>
#include <ion_efi/ion_efi.hpp>

class Kernel {
    void (* __attribute__((sysv_abi)) entry)(KernelArgs*);
    uint64_t kArgsAddr;
    indigo::Vec<KernelSegmentLoc> kSegs;
public:
    Kernel(const char* path, ion::RootNode* root, const char* trampPath);
    void Run(size_t argc, char** argv) __attribute__((noreturn));
};

#endif // !LOADER_KERNEL_HPP
