#ifndef LOADER_GRAPHICS_HPP
#define LOADER_GRAPHICS_HPP

#include <stdint.h>

struct FramebufferDescriptor {
  unsigned int* framebuffer = nullptr;
  unsigned int width = 0;
  unsigned int height = 0;
  unsigned int pitch = 0;
};

FramebufferDescriptor gopSetup();

#endif // !LOADER_GRAPHICS_HPP
