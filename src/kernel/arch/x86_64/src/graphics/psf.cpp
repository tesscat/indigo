#include <graphics/screen.hpp>
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

uint64_t cHeight;
uint64_t cWidth;

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

  cHeight = kargs->fbHeight/(font->height);
  cWidth = kargs->fbWidth/(font->width);

  return true;
}

uint64_t cx = 0;
uint64_t cy = 0;

void print(const char* str, uint32_t fg, uint32_t bg) {
  print(str, cx, cy, fg, bg);
}

void print(const char* str, uint64_t cx_, uint64_t cy_, uint32_t fg, uint32_t bg) {
  cx = cx_;
  cy = cy_;
  const char* pStr = str;
  while (*pStr) {
    putchar(*pStr, fg, bg);
    pStr++;
  }
}

void putchar(uint8_t c, uint32_t fg, uint32_t bg) {
  if (c == '\n') {
    cx = 0;
    cy++;
  } else {
    putchar(c, cx, cy, fg, bg);
    cx++;
    if (cx >= cWidth) {
      cx = 0;
      cy++;
    }
  }
  if (cy >= cHeight) {
    screen::scrollDown(height, bg);
    cy--;
  }

}

void putchar(uint8_t c, uint64_t cx_, uint64_t cy_, uint32_t fg, uint32_t bg) {
  putcharExactly(c, cx_*width, cy_*height, fg, bg);
}

void putcharExactly(uint8_t c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg) {
  uint8_t* glyph = (uint8_t*) &fontStart[(c > font->glyphCount ? 0 : c) * font->bytesPerGlyph];

  uint64_t x_, y_;
  int mask = 0;
  
  for (y_ = 0; y_ < height; y_++) {
    mask = 1<<(width-1);
    for (x_ = 0; x_ < width; x_++) {
      screen::plotPixel(x + x_, y + y_, (*glyph & mask) != 0 ? fg : bg);
      mask >>= 1;
    }

    glyph += bytesPerLine;
  }
}
}
