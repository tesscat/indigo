#ifndef LOADER_GRAPHICS_HPP
#define LOADER_GRAPHICS_HPP

#include <stdint.h>

// the second R stands for Reserved <3
enum PixelFormat {
    RGBR,
    BGRR,
};

struct FramebufferDescriptor {
    unsigned int* framebuffer = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int pitch = 0;
    PixelFormat format;
};

FramebufferDescriptor gopSetup();

#endif // !LOADER_GRAPHICS_HPP
