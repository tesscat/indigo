#define DEFAULT_FG 8

#include <drivers/vga/framebuffer.hpp>
#include <drivers/io/serial.hpp>

#define std nowedontneedit

extern "C" {
void kernelStart() {
  drivers::vga::printCStr("oi mate");
  drivers::io::serialConfigure(drivers::io::COM1);
  drivers::io::serialWrite("hello?");
  while (true) {}
  return;
}
}
