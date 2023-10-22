#ifndef DRIVERS_MEM_LGDT_HPP
#define DRIVERS_MEM_LGDT_HPP

#include <drivers/mem/gdt.hpp>

extern "C" {
// the overwhelming urge to s/d/b
void lgdt(drivers::mem::gdt_ptr* gdt);
}

#endif
