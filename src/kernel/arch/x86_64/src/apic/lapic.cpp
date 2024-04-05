#include "acpi/acpi.hpp"
#include "graphics/psf.hpp"
#include "util/util.hpp"
namespace apic {
void initLapic() {
    // find the LAPIC table
    acpi::MADT* madt = (acpi::MADT*)acpi::findTable("APIC");
    acpi::MADTHeader* curr = madt->contents;
    while ((uint64_t)curr < ((uint64_t)madt)+madt->header.Length) {
        switch (curr->type) {
            case acpi::ProcessorLocalApic : {
                graphics::psf::print("PLA\n");
                break;
            }
            case acpi::IOApic : {
                graphics::psf::print("IOA\n");
                break;
            }
            case acpi::IOApicSourceOverride : {
                graphics::psf::print("IOASO\n");
                break;
            }
            case acpi::IOApicNonMaskableSource : {
                graphics::psf::print("IOANMS\n");
                break;
            }
            case acpi::LocalApicNonMaskableSource : {
                graphics::psf::print("LANMS\n");
                break;
            }
            case acpi::LocalApicAddressOverride : {
                graphics::psf::print("LAAO\n");
                break;
            }
            case acpi::ProcessorLocalx2Apic : {
                graphics::psf::print("PL2A\n");
                break;
            }
            default : {
                panic("Unrecognised MADT type");
                break;
            }
        }
        *((uint64_t*)&curr) += curr->length;
    }
}
}
