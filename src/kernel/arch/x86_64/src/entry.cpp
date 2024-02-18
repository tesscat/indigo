#include "libstd/merge_sort.hpp"
#include "loader/memory_descriptor.hpp"
#include "memory/system_map.hpp"
#include <graphics/psf.hpp>
#include <graphics/screen.hpp>
#include <loader/kernel_args.hpp>
#include <libstd/itoa.hpp>

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;

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

extern "C" void _start(KernelArgs* args) {
    kargs = args;
    graphics::psf::initPSF();
    graphics::screen::initScreen();
    for (unsigned int x = 0; x < kargs->fbWidth; x++) {
        for (unsigned int y = 0; y < kargs->fbHeight; y++) {
            unsigned int pixel = ((255*x)/kargs->fbWidth & 0xFF) | 0xFF | ((((255*y)/kargs->fbHeight) << 8) & 0xFF00) | 0x00FF0000;
            graphics::screen::plotPixel(x, y, pixel);
        }
    }
    if (!args) {
        graphics::psf::print("Please give me arguments as kernel params!!");
        loop_forever;
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
        // if (t == BootServicesCode || t == BootServicesData) continue;
        // if (i%2 == 0) graphics::psf::print("\n");
        // else graphics::psf::print("               ");
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

    memory::SystemMap map {args->memDesc, static_cast<size_t>(args->memDescCount), args->kernelSegments, static_cast<size_t>(args->kernelSegmentsCount)};
    
    // okay. we are _currently_ identity-mapped with paging.
    // what we want to do is remap the next kernel to like 0x0xf00000000000
    // lets do some exploring of the current page table
    //
    // get CR3, the address of the current page table
    // uint64_t i;
    // asm("\t movq %%cr3,%0" : "=r"(i));
    // we are probably four-level page'd
    

    // lets build a page table that:
    // - maps the next loader s


    loop_forever;
}
