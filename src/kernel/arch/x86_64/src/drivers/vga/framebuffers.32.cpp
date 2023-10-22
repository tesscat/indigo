#include "drivers/io/ports.hpp"
#include <drivers/vga/framebuffer.hpp>

namespace drivers::vga {
#define VGA_BUFF 0xB8000
uint16_t* terminal_buffer = (uint16_t*)VGA_BUFF;
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

inline uint8_t combineColours(uint8_t fg, uint8_t bg) {
  return fg | bg << 4;
}
inline uint16_t generateEntry(char c, uint8_t colour) {
  return (uint16_t)c | (uint16_t)colour << 8;
}

void setChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg) {
  const uint32_t index = y*VGA_WIDTH + x;
  const uint16_t entry = generateEntry(c, combineColours(fg, bg));
  terminal_buffer[index] = entry;
}

uint32_t x_pos = 0;
uint32_t y_pos = 0;

inline void newline() {
  x_pos = 0;
  y_pos++;
}

void printChar(char c, uint8_t fg, uint8_t bg) {
  if (c == '\n') {
    newline();
    return;
  }
  setChar(x_pos, y_pos, c, fg, bg);

  x_pos++;
  if (x_pos >= VGA_WIDTH) {
    newline();
  }
}

#define VGA_FB_COMMAND_PORT 0x3D4
#define VGA_FB_DATA_PORT 0x3D5
#define VGA_FB_HIGH_BYTE_COMMAND 14
#define VGA_FB_LOW_BYTE_COMMAND 15

void moveCursor(uint32_t x, uint32_t y) {
  const uint16_t index = y*VGA_WIDTH + x;
  io::outB(VGA_FB_COMMAND_PORT, VGA_FB_HIGH_BYTE_COMMAND);
  io::outB(VGA_FB_DATA_PORT, ((index >> 8) & 0x00FF));
  io::outB(VGA_FB_COMMAND_PORT, VGA_FB_LOW_BYTE_COMMAND);
  io::outB(VGA_FB_DATA_PORT, ((index) & 0x00FF));
}

void printCStr(const char* str, uint8_t fg, uint8_t bg) {
  for (uint32_t index = 0; str[index]; index++) {
    printChar(str[index], fg, bg);
  }
  moveCursor(x_pos, y_pos);
}

}
