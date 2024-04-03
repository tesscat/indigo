#ifndef LOADER_ACPI_HPP
#define LOADER_ACPI_HPP

#include <uefi.h>

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

struct ACPIHeader_t {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed));

struct XSDT_t {
    ACPIHeader_t header;
    uint64_t entries[];
} __attribute__ ((packed));

#endif // !LOADER_ACPI_HPP
