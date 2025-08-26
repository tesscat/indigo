#ifndef KERNEL_APIC_IOAPIC_HPP
#define KERNEL_APIC_IOAPIC_HPP

#include <stdint.h>

namespace apic {
    void initIOApic();

    struct IOApic {
        volatile uint32_t ioRegSel;
        uint32_t padding[3];
        volatile uint32_t ioRegValue;
    } __attribute__((packed));

    extern IOApic* ioapic;

    struct RedirectionEntry {
        uint8_t vector : 8;
        // 000 = fixed, 001 = lowestPriority, 010 = SML, 100 = NMI, 101=INIT,
        // 111=ExtINIT
        uint8_t deliveryMode : 3;
        // 0 = physical dest, 1 = logical dest
        uint8_t destinationMode : 1;
        // 0 = irq is relaxed (nothing is happening), 1 = irq is sent but
        // awaiting delivery
        uint8_t deliveryStatus : 1;
        uint8_t remoteIRR      : 1;
        // 0 = edge, 1 = level, assume edge
        uint8_t triggerMode : 1;
        // i can temporarily mask the irq
        uint8_t mask        : 1;
        uint64_t padding    : 39 = 0;
        uint8_t destination : 8;
    } __attribute__((packed));

    void setRedirectionEntry(uint8_t index, RedirectionEntry entry);

}

#endif // !KERNEL_APIC_IOAPIC_HPP
