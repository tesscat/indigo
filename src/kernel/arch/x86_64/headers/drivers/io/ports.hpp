#ifndef DRIVERS_IO_PORTS_HPP

#include <common/types.hpp>

namespace drivers::io {

extern "C" {
/// Defined in ports.s
void outB(uint16_t port, uint8_t data);
uint8_t inB(uint16_t port);
}
}

#endif // !DRIVERS_IO_PORTS_HPP
