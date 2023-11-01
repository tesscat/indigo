#include "graphics/pixel.hpp"
#include <loader/kernel_args.hpp>
#include <graphics/psf.hpp>
#include <stdint.h>

extern KernelArgs* kargs;

namespace graphics::psf {
PSFFont* font;
uint32_t width;
uint32_t height;
uint64_t bytesPerLine = 0;
uint8_t* fontStart;

bool initPSF() {
  font = (PSFFont*)&_binary_font_psfu_start;
  if (font->magic != PSF_MAGIC) {
    return false;
  }
  height = font->height;
  width = font->width;
  
  // add 7 to round up
  bytesPerLine = (font->width+7)/8;

  fontStart = (uint8_t*)font + font->headerSize;

  return true;
}

void putchar(uint16_t c, uint64_t cx, uint64_t cy, uint32_t fg, uint32_t bg) {
}

void putcharExactly(uint16_t c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg) {
  uint8_t* glyph = (uint8_t*) &fontStart[(c > font->glyphCount ? 0 : c) * font->bytesPerGlyph];

  uint64_t x_, y_;
  int mask = 0;
  
  for (y_ = 0; y_ < height; y_++) {
    mask = 1<<(width-1);
    for (x_ = 0; x_ < width; x_++) {
      plotPixel(x + x_, y + y_, (*glyph & mask) != 0 ? fg : bg);
      mask >>= 1;
    }

    glyph += bytesPerLine;
  }
}
}
