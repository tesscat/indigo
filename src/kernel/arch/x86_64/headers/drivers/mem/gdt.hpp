#ifndef DRIVERS_MEM_GDT_HPP
#define DRIVERS_MEM_GDT_HPP

#include <common/types.hpp>

namespace drivers::mem {
struct gdt_ptr {
  uint16_t size;
  uint16_t addr;
} __attribute__((packed));
struct gdt_entry_unencoded {
  uint32_t limit;
  uint32_t base;
  uint8_t access;
  uint8_t flags;
};
uint8_t* encodeGdtEntry(uint8_t* target, gdt_entry_unencoded source);

struct segment_selector {
  uint8_t rpl : 2 = 0;
  bool is_local : 1 = 0;
  uint16_t offset : 13;
} __attribute__((packed));
}

#endif // !DRIVERS_MEM_GDT_HPP
