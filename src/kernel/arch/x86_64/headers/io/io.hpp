#ifndef KERNEL_IO_HPP
#define KERNEL_IO_HPP

#include <stdint.h>

namespace io {
inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}
}

#endif // !KERNEL_IO_HPP
