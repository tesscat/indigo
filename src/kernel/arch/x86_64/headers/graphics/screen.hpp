#ifndef KERNEL_GRAPHICS_PIXEL_HPP
#define KERNEL_GRAPHICS_PIXEL_HPP

#include <loader/kernel_args.hpp>
#include <stdint.h>

extern KernelArgs* kargs;
namespace graphics::screen {

    extern uint32_t* backBuffer;

    void initScreen();
    void plotPixel(unsigned int x, unsigned int y, unsigned int pixel);
    void scrollDown(unsigned int lines, uint32_t fill);
}
#endif // !KERNEL_GRAPHICS_PIXEL_HPP
