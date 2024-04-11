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
#include <multi/start_others.hpp>
#include <apic/ioapic.hpp>

KernelArgs* kargs;

// global ctors
using CtorFn = void(*)();
extern "C" CtorFn _init_array_start[];
extern "C" CtorFn _init_array_end[];

// Call _early_ setup (page tables, memory handling etc)
void kernel_initialize(KernelArgs* args) {
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
    memory::initSystemMap(args->memDesc, static_cast<size_t>(args->memDescCount));
    memory::initPhysAllocator();
    // immediately mark block for entry_others
    memory::markBlockAsUsed(0x8000, 4*KiB);
    memory::initPageAllocator();
    apic::pic::disablePic();
    apic::initGdt();
    apic::initIdt();
    memory::initHeap();
    acpi::initAcpi();
    apic::initLapic();
    apic::initIOApic();
}

void call_global_constructors() {
    for(CtorFn *fn = _init_array_start; fn != _init_array_end; fn++) {
        (*fn)();
    }
}

extern "C" void kernel_start(KernelArgs* args) {
    kernel_initialize(args);
    call_global_constructors();
    multi::startOthers();
    
    graphics::psf::consoleLock.lock();
    graphics::psf::print("kernel is finished :3\n");
    graphics::psf::consoleLock.release();

    loop_forever;

    unimplemented();
}

__attribute__ ((constructor)) void testctor() {
    graphics::psf::print("testctor called\n");
}

