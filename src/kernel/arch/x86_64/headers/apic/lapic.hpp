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

// struct LapicRegisters {
//     volatile uint32_t& lapicId;
//     volatile uint32_t const& lapicVersion;
//     volatile uint32_t& taskPriority;
//     volatile uint32_t const& arbitrationPriority;
//     volatile uint32_t const& processorPriority;
//     // write only
//     volatile uint32_t& eoi;
//     volatile uint32_t const& remoteRead;
//     volatile uint32_t& logicalDestination;
//     volatile uint32_t& destinationFormat;
//     volatile uint32_t& spuriousInterruptVector;
//     // volatile uint32_t const& inService;
//     // volatile uint32_t const& triggerModeRegister;
//     // volatile uint32_t const& interruptRequestRegister;
//     volatile uint32_t const& errorStatusRegister;
//     volatile lvtRegister& lvtCorrectedMachineCheckInterrupt;
//     // volatile uint32_t& interruptCommand;
//     volatile lvtRegister& lvtTimer;
//     volatile lvtRegister& lvtThermalSensor;
//     volatile lvtRegister& lvtPerformanceMonitoringCounters;
//     volatile lvtRegister& lint0;
//     volatile lvtRegister& lint1;
//     volatile lvtRegister& lvtError;
//     volatile uint32_t& timerInitialCount;
//     volatile uint32_t const& timerCurrentCount;
//     volatile uint32_t& timerDivideConfiguration;

//     #define helper_uint32(name, offset) name(*reinterpret_cast<uint32_t*>(base + offset))
//     #define helper_lvt(name, offset) name(*reinterpret_cast<lvtRegister*>(base + offset))
//     LapicRegisters(uint64_t base) :
//         helper_uint32(lapicId, 0x20),
//         helper_uint32(lapicVersion, 0x30),
//         helper_uint32(taskPriority, 0x80),
//         helper_uint32(arbitrationPriority, 0x90),
//         helper_uint32(processorPriority, 0x90),
//         helper_uint32(eoi, 0xB0),
//         helper_uint32(remoteRead, 0xC0),
//         helper_uint32(logicalDestination, 0xD0),
//         helper_uint32(destinationFormat, 0xE0),
//         helper_uint32(spuriousInterruptVector, 0xF0),
//         helper_uint32(errorStatusRegister, 0x280),
//         helper_lvt(lvtCorrectedMachineCheckInterrupt, 0x2F0),
//         helper_lvt(lvtTimer, 0x320),
//         helper_lvt(lvtThermalSensor, 0x220),
//         helper_lvt(lvtPerformanceMonitoringCounters, 0x340),
//         helper_lvt(lint0, 0x350),
//         helper_lvt(lint1, 0x360),
//         helper_lvt(lvtError, 0x370),
//         helper_uint32(timerInitialCount, 0x380),
//         helper_uint32(timerCurrentCount, 0x390),
//         helper_uint32(timerDivideConfiguration, 0x3E0)
//     {}
//     #undef helper_uint32
//     #undef helper_lvt
// };

}

#endif // !KERNEL_APIC_LAPIC_HPP
