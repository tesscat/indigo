#include "acpi/acpi.hpp"
#include "apic/idt.hpp"
#include "apic/gdt.hpp"
#include "apic/pic.hpp"
#include "libstd/merge_sort.hpp"
#include "memory/page_alloc.hpp"
#include "memory/system_map.hpp"
#include <graphics/psf.hpp>
#include <graphics/screen.hpp>
#include <loader/kernel_args.hpp>
#include <libstd/itoa.hpp>
#include <memory/phys_alloc.hpp>
#include <util/util.hpp>
#include <apic/lapic.hpp>
#include <memory/heap.hpp>

KernelArgs* kargs;

#ifdef DEBUG_OUTPUT
const char* mNames[] = {
    "ReservedMemoryType",
    "LoaderCode",
    "LoaderData",
    "BootServicesCode",
    "BootServicesData",
    "RuntimeServicesCode",
    "RuntimeServicesData",
    "ConventionalMemory",
    "UnusableMemory",
    "ACPIReclaimMemory",
    "ACPIMemoryNVS",
    "MemoryMappedIO",
    "MemoryMappedIOPortSpace",
    "PalCode",
    "PersistentMemory",
    "UnacceptedMemoryType",
    "MaxMemoryType"
};
#endif

__attribute__ ((interrupt)) void testInterrupt(apic::InterruptFrame* stack_frame) {
    graphics::psf::print("TEST INT WOOP\n");
    return;
}

__attribute__ ((interrupt)) void gpfStub(apic::InterruptFrame* frame, uint64_t code) {
    loop_forever;
    return;
}

extern "C" void kernel_start(KernelArgs* args) {
    kargs = args;
    graphics::psf::initPSF();
    graphics::screen::initScreen();
    for (unsigned int x = 0; x < kargs->fbWidth; x++) {
        for (unsigned int y = 0; y < kargs->fbHeight; y++) {
            unsigned int pixel = 0x0000ff00 | ((((255*y)/kargs->fbHeight) & 0xff ) << 24);
            graphics::screen::plotPixel(x, y, pixel);
        }
    }

    if (!kargs->fbIsBGR) {
        graphics::psf::print("NOT BGR reported");
    }

    imsort(args->memDesc, 0, args->memDescCount);


    #ifdef DEBUG_OUTPUT
    graphics::psf::print("Kernel arguments:");
    for (int i = 0; i < args->argc; i++) {
        graphics::psf::putchar(' ');
        graphics::psf::print(args->argv[i]);
    }

    char sp[16];
    int i = itoa(args->memDescCount, sp);
    sp[i] = '\0';

    graphics::psf::print("\nMem desc count: ");
    graphics::psf::print(sp);

    graphics::psf::print("\nStart | End | type");

    
    // there seems to be a garbage mDesc with base 0x0
    for (int i = 0; i < args->memDescCount; i++) {
        if (args->memDesc[i].physStart == 0x0) continue;
        int t = args->memDesc[i].type;
        graphics::psf::print("\n");
        int l = itoa(args->memDesc[i].physStart, sp, 16);
        sp[l] = '\0';
        graphics::psf::print("0x");
        for (int j = 0; j < (16-l); j++) graphics::psf::print("0");
        graphics::psf::print(sp);
        int end = args->memDesc[i].physStart + 4*1024*args->memDesc[i].nrPages;
        l = itoa(end, sp, 16);
        sp[l] = '\0';
        graphics::psf::print(" | 0x");
        for (int j = 0; j < (16-l); j++) graphics::psf::print("0");
        graphics::psf::print(sp);
        if (args->memDesc[i].type >= MaxMemoryType || args->memDesc[i].type < 0) {
            graphics::psf::print(" | Invalid");
        } else {
            graphics::psf::print(" | ");
            graphics::psf::print(mNames[args->memDesc[i].type]);
        }
    }
    #endif

    memory::initSystemMap(args->memDesc, static_cast<size_t>(args->memDescCount));

    memory::initPhysAllocator();

    memory::initPageAllocator();

    memory::initHeap();

    apic::pic::disablePic();
    apic::initGdt();
    apic::initIdt();

    acpi::initAcpi();

    apic::initLapic();

    graphics::psf::print("woop");

    void* test = kmalloc(7483);
    void* test2 = kmalloc(23);
    kfree(test);
    void* test3 = kmalloc(8000);
    void* test4 = kmalloc(28);

    unimplemented();
}

