#ifndef KERNEL_ACPI_ACPI_HPP
#define KERNEL_ACPI_ACPI_HPP

#include <loader/acpi_partial.hpp>

namespace acpi {
    ACPIHeader* findTable(const char sig[4]);
    void initAcpi();

    // some table defs
    enum MADTEntryType {
        ProcessorLocalApic         = 0,
        IOApic                     = 1,
        IOApicSourceOverride       = 2,
        IOApicNonMaskableSource    = 3,
        LocalApicNonMaskableSource = 4,
        LocalApicAddressOverride   = 5,
        ProcessorLocalx2Apic       = 9,
    };
    struct MADTHeader {
        MADTEntryType type : 8;
        uint8_t length;
    } __attribute__((packed));
    struct MADT {
        ACPIHeader header;
        uint32_t localAddr;
        uint32_t flags;
        MADTHeader contents[];
    } __attribute__((packed));

}

#endif // !KERNEL_ACPI_ACPI_HPP
