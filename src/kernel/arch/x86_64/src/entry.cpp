#include <graphics/psf.hpp>
#include <graphics/screen.hpp>
#include <loader/kernel_args.hpp>

KernelArgs* kargs;

/**
 * Example "kernel"
 */
extern "C" void _start(KernelArgs* args) {
  kargs = args;
  graphics::psf::initPSF();
  graphics::screen::initScreen();
  for (unsigned int x = 0; x < kargs->fbWidth; x++) {
    for (unsigned int y = 0; y < kargs->fbHeight; y++) {
      unsigned int pixel = ((255*x)/kargs->fbWidth & 0xFF) | 0xFF | ((((255*y)/kargs->fbHeight) << 8) & 0xFF00) | 0x00FF0000;
      graphics::screen::plotPixel(x, y, pixel);
    }
  }

  for (uint64_t i = 0; i < 50; i++) {
    graphics::psf::putchar('A' + i, 0x00FFFFFF, 0x00000000);
    graphics::psf::putchar('\n', 200, 0);
  }

  // graphics::psf::putcharExactly('~', 20, 207, 0x00FFFFFF, 0x00000000);
}
//
// inline void plotPixel(uint32_t x, uint32_t y, uint32_t* fb, uint32_t pitch, uint32_t pixel) {
//   fb[(y*pitch + x)] = pixel;
// }
//
// extern "C" void _start(KernelArgs* args) {
//   for (uint32_t i = 0; i < 7; i++) {
//     for (uint32_t j = 0; j < 10; j++)
//       plotPixel(j, i, args->framebuffer, args->fbPitch, 0xFF);
//   }
//   // args->framebuffer[6*args->fbPitch + 3] = 0xFFFFFFFF;
//     // plotPixel(2, 2, args->framebuffer, args->fbPitch, 0xFFFFFFFF);
//   // plotPixel(0, 0, args->framebuffer, args->fbPitch, 0xffffffff);
//   // for (size_t y = 0; y < args->fbHeight; y++) {
//     // for (size_t x = 0; x < args->fbWidth; x++) {
//       // uint8_t r = (y*255)/args->fbHeight;
//       // uint8_t g = (x*255)/args->fbWidth;
//       // uint32_t pixel = (((uint32_t)r) << 0 & 0x000000FF) | (((uint32_t)g) << 16 & 0x0000FF00) | 0x00FF0000;
//       // uint32_t pixel = 0x00FFFFFFFF;
//       // plotPixel(x, y, args->framebuffer, args->fbPitch, pixel);
//     // }
//     // while(true) {};
//   // }
//   // if (args->fbWidth != 1280 || args->fbPitch != 1280 || args->fbHeight != 800 || (uint64_t)args->framebuffer != 0x80000000)
//     // while(1);
//   // args->framebuffer = (uint32_t*)0x80000000;
//   // for (uint64_t i = 0; i < (args->fbPitch * args->fbHeight); i++) {
//     // args->framebuffer[i] = 0xFFFFFFFF;
//   // }
//   while(1);
// }
