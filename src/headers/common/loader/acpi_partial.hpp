#ifndef COMMON_LOADER_ACPI_PARTIAL_HPP
#define COMMON_LOADER_ACPI_PARTIAL_HPP

#include <stdint.h>

namespace acpi {
struct ACPIHeader {
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

struct XSDT {
    ACPIHeader header;
    uint64_t entries[];
} __attribute__ ((packed));
}

#endif // !COMMON_LOADER_ACPI_PARTIAL_HPP
