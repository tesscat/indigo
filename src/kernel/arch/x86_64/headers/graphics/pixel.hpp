#ifndef KERNEL_GRAPHICS_PIXEL_HPP
#define KERNEL_GRAPHICS_PIXEL_HPP

#include <loader/kernel_args.hpp>

extern KernelArgs* kargs;

inline void plotPixel(unsigned int x, unsigned int y, unsigned int pixel) {
  kargs->framebuffer[x + y*kargs->fbPitch] = pixel;
}

#endif // !KERNEL_GRAPHICS_PIXEL_HPP
