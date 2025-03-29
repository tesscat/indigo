#include "graphics/screen.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include <util/util.hpp>
#include <libmem/string.hpp>
#include <acpi/acpi.hpp>

namespace acpi {
uint64_t xsdt_entries;

void initAcpi() {
    // map the ACPI table so we can acc read it
    XSDT* xsdt = kargs->xsdt;
    uint32_t size = 4*KiB;

    memory::kernelIdentityMap4KiBBlock((uint64_t)xsdt);
    // map more if we need it
    if (xsdt->header.Length > 4*KiB) {
        size = xsdt->header.Length;
        // unmap then re-map so we can get 2MiB action if we need it
        memory::kernelUnmap4KiBBlock((uint64_t)xsdt);
        memory::kernelIdentityMapBlock((uint64_t)xsdt, size);
    }
    memory::markBlockAsUsed((uint64_t)xsdt, size);
    xsdt_entries = (xsdt->header.Length - sizeof(ACPIHeader))/sizeof(uint64_t);

    // pre-emptively map all the tables since it'll be nicer
    for (uint64_t i = 0; i < xsdt_entries; i++) {
        ACPIHeader* header = (ACPIHeader*)xsdt->entries[i];
        memory::kernelIdentityMap4KiBBlock((uint64_t)header);
        // size shenanigans
        size = 4*KiB;
        if (header->Length > 4*KiB) {
            size = header->Length;
            memory::kernelUnmap4KiBBlock((uint64_t)header);
            memory::kernelIdentityMapBlock((uint64_t)header, size);
        }
        memory::markBlockAsUsed((uint64_t)header, size);
    }
}

acpi::ACPIHeader* findTable(const char sig[4]) {
    for (uint64_t i = 0; i < xsdt_entries; i++) {
        ACPIHeader* header = (ACPIHeader*)kargs->xsdt->entries[i];
        if (libmem::strncmp(sig, header->Signature, 4))
            return header;
    }
    return nullptr;
}
}
