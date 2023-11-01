#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP


struct KernelArgs {
  unsigned int* framebuffer = nullptr;
  unsigned int fbWidth = 0;
  unsigned int fbHeight = 0;
  unsigned int fbPitch = 0;
  int argc = 0;
  unsigned int totalSize = 0;
  char** argv = nullptr;
};

#endif // !COMMON_LOADER_KERNEL_ARGS_HPP
