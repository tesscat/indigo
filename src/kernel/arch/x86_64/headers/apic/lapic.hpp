#ifndef KERNEL_APIC_LAPIC_HPP
#define KERNEL_APIC_LAPIC_HPP

#include "acpi/acpi.hpp"
namespace apic {
void initLapic();

struct ProcessorLocalApic {
    acpi::MADTHeader header;
    uint8_t acpiProcessorId : 8;
    uint8_t apicId : 8;
    uint32_t flags : 32;
} __attribute__ ((packed));
struct IOApic {
    acpi::MADTHeader header;
    uint8_t ioApicId : 8;
    uint8_t reserved : 8;
    uint32_t ioApicAddress : 32;
    uint32_t globalSystemInterruptBase : 32;
} __attribute__ ((packed));
struct IOApicSourceOverride {
    acpi::MADTHeader header;
    uint8_t busSource : 8;
    uint8_t irqSource : 8;
    uint32_t globalSystemInterrupt : 32;
    uint16_t flags : 16;
} __attribute__ ((packed));
struct IOApicNonMaskableSource {
    acpi::MADTHeader header;
    uint8_t nmiSource : 8;
    uint8_t reserved : 8;
    uint16_t flags : 16;
    uint32_t globalSystemInterrupt : 32;
} __attribute__ ((packed));
struct LocalApicNonMaskableSource {
    acpi::MADTHeader header;
    uint8_t acpiProcessorId: 8;
    uint16_t flags : 16;
    uint8_t lint : 8;
} __attribute__ ((packed));
struct LocalApicAddressOverride {
    acpi::MADTHeader header;
    uint64_t physAddr : 64;
} __attribute__ ((packed));
struct ProcessorLocalx2Apic {
    acpi::MADTHeader header;
    uint16_t reserved : 16;
    uint32_t apicId : 32;
    uint32_t flags : 32;
    uint32_t acpiProcessorId : 32;
} __attribute__ ((packed));

struct LvtRegister {
    uint8_t vectorNumber : 8;
    // TODO: what is this
    uint8_t reserved : 3;
    uint8_t reserved2 : 1;
    uint8_t isInterruptPending : 1;
    uint8_t isLowTriggered : 1;
    uint8_t remoteIrr : 1;
    uint8_t isLevelTriggered : 1;
    uint8_t isMasked : 1;
    uint16_t reserved3 : 15;
    inline void clear() volatile {
        // we are 32bit
        *(uint32_t*)(this) = 0;
    }
} __attribute__ ((packed));

void setApicTimerHz(uint64_t hz);
}

#endif // !KERNEL_APIC_LAPIC_HPP