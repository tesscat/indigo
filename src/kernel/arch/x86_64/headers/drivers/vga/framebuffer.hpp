#ifndef DRIVERS_VGA_HPP
#define DRIVERS_VGA_HPP

#include <common/types.hpp>

namespace drivers::vga {
#ifndef DEFAULT_BG
// black
#define DEFAULT_BG 0
#endif
#ifndef DEFAULT_FG
// white
#define DEFAULT_FG 15
#endif

void setChar(uint32_t x, uint32_t y, char c, uint8_t fg = DEFAULT_FG, uint8_t bg = DEFAULT_BG);

void printChar(char c, uint8_t fg = DEFAULT_FG, uint8_t bg = DEFAULT_BG);

void printCStr(const char* str, uint8_t fg = DEFAULT_FG, uint8_t bg = DEFAULT_BG);
}

#endif // !DRIVERS_VGA_HPP
