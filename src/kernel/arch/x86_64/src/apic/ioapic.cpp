#include "acpi/acpi.hpp"
#include "apic/lapic.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include <apic/ioapic.hpp>

namespace apic {

IOApic* ioapic;
uint8_t irqCount;

void initIOApic() {
    // loop over the MADT, picking out relevant IOAPIC stuff
    acpi::MADTHeader* curr = madt->contents;
    while ((uint64_t)curr < ((uint64_t)madt)+madt->header.Length) {
        switch (curr->type) {
            case acpi::IOApic : {
                tables::IOApic* ioa = (tables::IOApic*)curr;
                ioapic = (IOApic*)(uint64_t)ioa->ioApicAddress;
                break;
            }
            default : {
                break;
            }
        }
        *((uint64_t*)&curr) += curr->length;
    }
    memory::markBlockAsUsed((uint64_t)ioapic, sizeof(IOApic));
    memory::kernelIdentityMap4KiBBlock((uint64_t)ioapic);
    // how many interrupts can we handle?
    ioapic->ioRegSel = 1;
    irqCount = (ioapic->ioRegValue >> 16) & 0xff;
    // mark all interrupts as disabled
    RedirectionEntry entry {
        .vector=0,
        .deliveryMode=0,
        .destinationMode=0,
        .deliveryStatus=0,
        .remoteIRR=0,
        .triggerMode=0,
        // important line
        .mask=1,
        .destination=0,
    };
    for (int i = 0; i < irqCount; i++) {
        setRedirectionEntry(i, entry);
    }
}

void setRedirectionEntry(uint8_t index, RedirectionEntry entry) {
    // set low
    ioapic->ioRegSel = 0x10 + 2*index;
    ioapic->ioRegValue = (uint32_t)((*(uint64_t*)&entry) & 0xffffffff);
    // high
    ioapic->ioRegSel = 0x10 + 2*index + 1;
    ioapic->ioRegValue = (uint32_t)((*(uint64_t*)&entry) >> 32 & 0xffffffff);
}

}
