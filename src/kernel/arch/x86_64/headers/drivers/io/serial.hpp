#include <common/types.hpp>

namespace drivers::io {
const uint16_t COM1 = 0x3f8;

void serialWrite(const char* data, uint16_t com = COM1);
void serialConfigure(uint16_t com, uint16_t divisor = 4);
}
