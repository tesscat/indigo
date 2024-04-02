/// Find the ACPI tables, and fill out the relevant fields in the stuff
/// we will pass to the kernel

// https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#efi-system-table-1
#define EFI_ACPI_20_TABLE_GUID \
    {0x8868e871,0xe4f1,0x11d3,\
    {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}

#define EFI_ACPI_20_TABLE_GUID_T efi_guid_t EFI_ACPI_20_TABLE_GUID

#include "uefi.h"

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

void initAcpiTables() {
    void* acpi_table;
    bool found = false;
    for (int i = 0; i < ST->NumberOfTableEntries; i++) {
        // obtain the GUID
        efi_guid_t guid = ST->ConfigurationTable[i].VendorGuid;
        if (isAcpiGuid(guid)) {
            acpi_table = ST->ConfigurationTable[i].VendorTable;
            found = true;
            break;
        }
    }
    if (!found) {
        printf("Unable to find acpi table");
        while (1);
    }
}
