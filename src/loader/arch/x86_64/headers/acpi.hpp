#ifndef LOADER_ACPI_HPP
#define LOADER_ACPI_HPP

#include <stdint.h>
#define _STDINT_H
#include <uefi.h>
#include <loader/acpi_partial.hpp>

namespace acpi {
void initAcpiTables();
unsigned int countCpus();

// https://wiki.osdev.org/RSDP
struct XSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;      // deprecated since version 2.0
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

}

#endif // !LOADER_ACPI_HPP
