#ifndef KERNEL_GRAPHICS_PSH_HPP
#define KERNEL_GRAPHICS_PSH_HPP

#include "sync/spinlock.hpp"
#include <stdint.h>

extern char _binary_font_psfu_start[];
extern char _binary_font_psfu_end;


#define PSF_MAGIC 0x864ab572
#define PSF1_MAGIC 0x0436

namespace graphics::psf {
struct PSFFont {
  uint32_t magic;
  uint32_t version;
  uint32_t headerSize;
  // 1 if there's a unicode decoding table
  uint32_t flags;
  uint32_t glyphCount;
  uint32_t bytesPerGlyph;
  uint32_t height;
  uint32_t width;
};

extern sync::Spinlock consoleLock;

bool initPSF();
void putchar(uint8_t c, uint64_t cx, uint64_t cy, uint32_t fg, uint32_t bg);
void putcharExactly(uint8_t c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg);
void putchar(uint8_t c, uint32_t fg = 0x00FFFFFF, uint32_t bg = 0x00);
void print(const char* str, uint32_t fg = 0x00FFFFFF, uint32_t bg = 0x00);
void print(const char* str, uint64_t cx_, uint64_t cy_, uint32_t fg, uint32_t bg);

}

#endif // !KERNEL_GRAPHICS_PSH_HPP
