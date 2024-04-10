#ifndef KERNEL_APIC_LAPIC_HPP
#define KERNEL_APIC_LAPIC_HPP

#include "acpi/acpi.hpp"
namespace apic {
void initLapic();

#define LAPIC_BASE 0xFEE00000

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

void enableLapic();

void apicSleep(uint64_t ms);

namespace lapic {
#define helper_uint32(name, offs) extern uint32_t volatile* name;
#define helper_uint32_const(name, offs) extern uint32_t const volatile* name;
#define helper_lvt(name, offs) extern LvtRegister volatile* name;
helper_uint32(apicId, 0x20)
helper_uint32_const(apicVersion, 0x30)
helper_uint32(taskPriority, 0x80)
helper_uint32_const(arbitrationPriority, 0x90)
helper_uint32_const(processorPriority, 0xA0)
helper_uint32(eoi, 0xB0)
helper_uint32_const(remoteRead, 0xC0)
helper_uint32(logicalDestination, 0xD0)
helper_uint32(destinationFormat, 0xE0)
helper_uint32(spuriousInterruptVector, 0xF0)
// TODO: the In-Service register and Trigger Mode Register and Interrupt Request Register
helper_uint32(errorStatus, 0x280)
helper_uint32(icr0, 0x300)
helper_uint32(icr1, 0x310)
namespace lvt {
helper_lvt(correctedMachineCheck, 0x2F0)
helper_lvt(timer, 0x320)
helper_lvt(thermalSensor, 0x330)
helper_lvt(performanceMonitor, 0x340)
helper_lvt(lint0, 0x350)
helper_lvt(lint1, 0x360)
helper_lvt(error, 0x370)
}
helper_uint32(timerInitialCount, 0x380)
helper_uint32_const(timerCurrentCount, 0x390)
helper_uint32(timerDivideConfiguration, 0x3E0)
}
#undef helper_uint32_const
#undef helper_uint32
#undef helper_lvt
}

#endif // !KERNEL_APIC_LAPIC_HPP
