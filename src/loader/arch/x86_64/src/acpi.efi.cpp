/// Find the ACPI tables, and fill out the relevant fields in the stuff
/// we will pass to the kernel


#include <acpi.hpp>

namespace acpi {

// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-system-table-1
#define EFI_ACPI_20_TABLE_GUID \
{0x8868e871,0xe4f1,0x11d3,\
    {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}

#define EFI_ACPI_20_TABLE_GUID_T efi_guid_t EFI_ACPI_20_TABLE_GUID

bool isAcpiGuid(efi_guid_t guid) {
    if (guid.Data1 == EFI_ACPI_20_TABLE_GUID_T.Data1 
        && guid.Data2 == EFI_ACPI_20_TABLE_GUID_T.Data2
        && guid.Data3 == EFI_ACPI_20_TABLE_GUID_T.Data3
    ) {
        for (int i = 0; i < 8; i++) {
            if (guid.Data4[i] != EFI_ACPI_20_TABLE_GUID_T.Data4[i])
                return false;
        }
        // found it!
        return true;
    }
    return false;
}

XSDT *xsdt;
uint64_t xsdt_entries;

void* findTableInXSDT(const char sig[4]) {
    for (int i = 0; i < xsdt_entries; i++) {
        ACPIHeader* header = (ACPIHeader*)xsdt->entries[i];
        if (memcmp(header->Signature, sig, 4) == 0)
            return header;
    }
    return nullptr;
}

struct APICSubHeader_t {
    uint8_t entryType;
    uint8_t recordLength;
} __attribute__ ((packed));
struct APIC_t {
    ACPIHeader header;
    uint32_t localAPICAddress;
    // TODO: if flags == 1 mask 8259 PIC's interrupts
    uint32_t flags;
    APICSubHeader_t additionalData[];
} __attribute__ ((packed));

unsigned int countCpus() {
    // find the APIC
    APIC_t* apic = (APIC_t*) findTableInXSDT("APIC");
    if (!apic) {
        printf("Unable to find APIC");
        while(1);
    }
    uint64_t n_cpus = 0;
    APICSubHeader_t* curr = &apic->additionalData[0];
    while ((uint64_t)curr < ((uint64_t)apic + apic->header.Length)) {
        // what this header look like
        if (curr->entryType == 0) {
            // processor-local APIC
            n_cpus++;
        }
        if (curr->recordLength == 0) {
            printf("uh oh");
            while(1);
        }
        uint64_t rL = curr->recordLength;
        // TODO: why does /2 make it work?
        curr += rL/2;
    }

    return n_cpus;
    
}

void initAcpiTables() {
    XSDP_t* acpi_table;
    bool found = false;
    for (int i = 0; i < ST->NumberOfTableEntries; i++) {
        // obtain the GUID
        efi_guid_t guid = ST->ConfigurationTable[i].VendorGuid;
        if (isAcpiGuid(guid)) {
            acpi_table = (XSDP_t*) ST->ConfigurationTable[i].VendorTable;
            found = true;
            break;
        }
    }
    if (!found) {
        printf("Unable to find acpi table");
        while (1);
    }
    if (memcmp(acpi_table->Signature, "RSD PTR ", 8) != 0) {
        printf("Invalid RDSP sig");
        while (1);
    } 
    char buff[16];
    buff[4] = '\0';

    xsdt = (XSDT*)acpi_table->XsdtAddress;
    xsdt_entries = (xsdt->header.Length - sizeof(ACPIHeader))/sizeof(uint64_t);
}
}
