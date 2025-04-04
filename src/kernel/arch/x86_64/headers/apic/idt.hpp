#ifndef KERNEL_APIC_IDT_HPP
#define KERNEL_APIC_IDT_HPP

#include "apic/gdt.hpp"
#include <stdint.h>

namespace apic {
struct IdtGate {
    uint16_t offsetLow : 16;
    // segment selector
    SegmentSelector segsel;
    uint8_t ist : 3;
    uint8_t reserved : 5;
    uint8_t gateType : 4;
    uint8_t null : 1 = 0;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offsetMid : 16;
    uint32_t offsetHigh : 32;
    uint32_t reserved2 : 32;

    inline void setAddr(uint64_t addr) {
        offsetLow = addr & 0xffff;
        offsetMid = (addr >> 16) & 0xffff;
        offsetHigh = (addr >> 32) & 0xffffffff;
    }
    inline void clear() {
        // we're 16 bytes, or 2x64bits
        *(uint64_t*)(this) = 0;
        *(uint64_t*)(((uint64_t)this) + 8) = 0;
    }
} __attribute__ ((packed));

struct IdtDescriptor {
    uint16_t size : 16;
    uint64_t offset : 64;
} __attribute__ ((packed));

struct InterruptFrame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} __attribute__ ((packed));

void initIdt();
void enableIdt();
// the backend is the same, just wanna make sure the user (me) isn't mixing up the function _types_,
// since clang _will_ complain and/or break IRQs

void registerExceptionHandler(uint64_t idx, void (*func)(InterruptFrame*, uint64_t), bool isTrap, uint8_t dpl = 0);
inline void registerInterruptHandler(uint64_t idx, void (*func)(InterruptFrame*), bool isTrap, uint8_t dpl = 0) {
    registerExceptionHandler(idx, reinterpret_cast<void(*)(InterruptFrame*, uint64_t)>(func), isTrap, dpl);
}

}

#endif // !KERNEL_APIC_IDT_HPP
