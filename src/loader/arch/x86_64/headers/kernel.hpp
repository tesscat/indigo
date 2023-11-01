#ifndef LOADER_LOAD_HPP
#define LOADER_LOAD_HPP

#include <loader/kernel_args.hpp>
#include <ion_efi/ion_efi.hpp>

class Kernel {
  void (* __attribute__((sysv_abi)) entry)(KernelArgs*);
public:
  Kernel(const char* path, ion::RootNode* root);
  void Run(size_t argc, char** argv) __attribute__((noreturn));
};

#endif // !LOADER_LOAD_HPP
