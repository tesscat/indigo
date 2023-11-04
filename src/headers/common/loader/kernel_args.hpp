#ifndef COMMON_LOADER_KERNEL_ARGS_HPP
#define COMMON_LOADER_KERNEL_ARGS_HPP


struct KernelArgs {
  unsigned int* framebuffer = nullptr;
  unsigned int fbWidth;
  unsigned int fbHeight;
  unsigned int fbPitch;
  int argc = 0;
  unsigned int totalSize;
  char** argv = nullptr;
};

#endif // !COMMON_LOADER_KERNEL_ARGS_HPP
