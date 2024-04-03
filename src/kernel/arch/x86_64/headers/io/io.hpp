#ifndef KERNEL_IO_HPP
#define KERNEL_IO_HPP

#include <stdint.h>

namespace io {
static inline void outb(uint64_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

}

#endif // !KERNEL_IO_HPP
