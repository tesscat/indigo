#include "apic/gdt.hpp"

namespace apic {
__attribute__((aligned(8)))
GdtPointer pointer;
__attribute__((aligned(8)))
GdtDescriptor gdt[8];

extern "C" void reloadSegments(void);

void initGdt() {
    // empty it
    for (int i = 0; i < 8; i++) {
        gdt[i].clear();
    }
    // null seg
    // <already cleared>
    // kernel mode code segment
    // present, dpl 0, not tss, executable, rw, accessed
    gdt[1].accessByte = 0b10011011;
    // long mode code
    gdt[1].flags = 0b0010;

    // kernel mode data
    // present, dpl 0, not tss, not exec, rw, accessed
    gdt[2].accessByte = 0b10010011;
    gdt[2].flags = 0b0000;

    // user mode code
    // present, dpl 3, not tss, executable, rw, accessed
    gdt[3].accessByte = 0b11111011;
    gdt[3].flags = 0b0010;

    // user mode data
    // present, dpl 3, not tss, not exec, rw, accessed
    gdt[4].accessByte = 0b11110011;
    gdt[4].flags = 0b0000;
    // TODO: TSS for setting interrupt stack pointers
    // TODO: sysenter/sysexit setup

    // set it up
    __asm__ volatile ("cli");

    // uint64_t* gdtU = (uint64_t*)gdt;
    // gdtU[1] = 0x00af9a000000ffff;
    // gdtU[2] = 0x00cf92000000ffff;

    pointer.size = sizeof(GdtDescriptor) * 5;
    pointer.offset = (uint64_t)&gdt[0];
    __asm__ volatile ("lgdt %0" : : "m"(pointer));

    reloadSegments();
}
}
