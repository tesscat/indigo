#include "acpi.hpp"
#include "loader/memory_descriptor.hpp"
#include "memmap.hpp"
#include <stdint.h>
#define _STDINT_H
#include <uefi.h>
#include <elf.hpp>
#include <file.hpp>
#include <ion_efi/ion_efi.hpp>
#include <kernel.hpp>
#include <loader/kernel_args.hpp>
#include <graphics.hpp>

namespace acpi {
extern acpi::XSDT *xsdt;
}

#define loop_forever while(1){__asm__ volatile ("hlt");}

Kernel::Kernel(const char* path, ion::RootNode* root, const char* trampPath, const char* fsDriverPath, const char* initrdPath) {
    // load us up on kernel
    BinaryFile file = readBinaryFile(path);
    if (file.data == nullptr) {
        printf("Kernel not found or unreadable");
        loop_forever;
        return;
    }


    kElf = (uint8_t*)file.data;
    kElfLen = file.len;


    // are we ELF?
    elf::ElfHeader* header = (elf::ElfHeader*) file.data;
    if (!elf::isUsableElfExeHeader(header)) {
        printf("unusable kernel, invalid ELF header");
        loop_forever;
        return;
    }


    uint64_t maxKAddr = 0;

    // load kernel segments
    for (int i = 0; i < header->progHeaderTableLength; i++) {
        elf::ProgHeader* prog_header = (elf::ProgHeader*)((uint8_t*)file.data + header->progHeaderPos + i*header->progEntrySize);
        if (prog_header->type == PH_LOAD) {
            printf("current pA is 0x%x\n", prog_header->physAddr);
            memcpy((void*)prog_header->physAddr, (uint8_t*)file.data + prog_header->fileOffset, prog_header->fileSize);
            memset(((uint8_t*)prog_header->physAddr + prog_header->fileSize), 0, prog_header->memSize - prog_header->fileSize);
            uint64_t mAddr = prog_header->physAddr + prog_header->memSize;
            if (mAddr > maxKAddr) maxKAddr = mAddr;

            kSegs.Append(KernelSegmentLoc {.physLoc = prog_header->physAddr, .len = prog_header->memSize});
        }
    }
    // load the whole of fsDriver into RAM for the kernel
    BinaryFile fsDriver = readBinaryFile(fsDriverPath);
    if (fsDriver.data == nullptr) {
        printf("FsDriver not found or unreadable");
        loop_forever;
        return;
    }
    fsD = (uint8_t*)fsDriver.data;
    fsDLen = fsDriver.len;


    // load the whole initrd into RAM for the kernel
    BinaryFile initrd = readBinaryFile(initrdPath);
    if (initrd.data == nullptr) {
        printf("initrd not found or unreadable");
        loop_forever;
        return;
    }
    initrdData = (uint8_t*)initrd.data;
    initrdLen = initrd.len;


    // pick the addr for kArgs as above the end of the kernel
    // we really should page-align it so we can mark not-exe in the future but that's future me problem
    // TODO: see above
    // pad it out a little since kernel __attribute__ ((align)) is a bit dodgy and likes to trample me
    // give it a KiB
    // in future, if something pagefaults randomly it's probably kargs->framebuffer being trampled, so increase this value
    kArgsAddr = maxKAddr + (1024);
    printf("Max kAddr was %x\n", maxKAddr);


    // load trampoline segments
    BinaryFile tFile = readBinaryFile(trampPath);
    if (tFile.data == nullptr) {
        printf("Trampoline not found or unreadable");
        loop_forever;
        return;
    }
    // are we ELF?
    elf::ElfHeader* tHeader = (elf::ElfHeader*) tFile.data;
    if (!elf::isUsableElfExeHeader(tHeader)) {
        printf("unusable trampoline, invalid ELF header");
        loop_forever;
        return;
    }

    // load segments
    for (int i = 0; i < tHeader->progHeaderTableLength; i++) {
        elf::ProgHeader* prog_header = (elf::ProgHeader*)((uint8_t*)tFile.data + tHeader->progHeaderPos + i*tHeader->progEntrySize);
        if (prog_header->type == PH_LOAD) {
            memcpy((void*)prog_header->physAddr, (uint8_t*)tFile.data + prog_header->fileOffset, prog_header->fileSize);
            memset(((uint8_t*)prog_header->physAddr + prog_header->fileSize), 0, prog_header->memSize - prog_header->fileSize);
        }
    }


    void* entry_loc = (void*)tHeader->entryPointPos;
    entry = (void (* __attribute__((sysv_abi)))(KernelArgs*, uint64_t)) entry_loc;

    kEntry = header->entryPointPos;

    delete (uint8_t*)tFile.data;
}

void Kernel::Run(size_t argc, char** argv) {
    // calculate the size of KernelArgs
    size_t size = sizeof(KernelArgs);
    // add on the bit for the char*-s
    size += argc*sizeof(char*);
    // add on for each of the strings themselves
    for (size_t i = 0; i < argc; i++) {
        // plus null-termi
        size += strlen(argv[i]) + 1;
    }
    
    // handle the memmap first
    size_t mmapOrigSize = getMemmapSize();
    // TODO: acc figure this line out. since it's not 8, and one memory alloc isn't the size of efi_memory_descriptor_t
    uint8_t* mmapOrig = (uint8_t*)malloc(mmapOrigSize + sizeof(efi_memory_descriptor_t)*8);
    MemMapInfo origInfo = getMemmap(mmapOrig);
    int mmapLen = origInfo.fullSize/origInfo.descriptorSize;
    uint64_t mmapSize = mmapLen*sizeof(MemoryDescriptor);


    size += mmapSize;

    size_t kSegSize = sizeof(KernelSegmentLoc)*(kSegs.len + 1);
    
    // add the kernel segments, plus one for kargs itself
    
    size += kSegSize;

    // ok allocate it
    // turns out we cheatin. just chuck it a bit above the kernel
    KernelArgs* args = (KernelArgs*) kArgsAddr;
    args->totalSize = size;

    // count CPUS
    args->n_cpus = acpi::countCpus();
    // xsdt pointer
    args->xsdt = acpi::xsdt;

    // do the graphics
    FramebufferDescriptor fb = gopSetup();
    // do the struct-filling
    args->framebuffer = fb.framebuffer;
    args->fbWidth = fb.width;
    args->fbHeight = fb.height;
    args->fbPitch = fb.pitch;
    args->fbIsBGR = fb.format == BGRR;
    
    // fsDriver
    args->fsDriver = fsD;
    args->fsDriverLen = fsDLen;
    // initrd
    args->initrd = initrdData;
    args->initrdLen = initrdLen;
    // kElf
    args->kElf = kElf;
    args->kElfLen = kElfLen;

    // string wrangling, fun times
    char** end = (char**) ((uint8_t*)args + size);
    args->argv = end;
    args->argc = argc;
    // copy over the char*s
    memcpy(end, argv, sizeof(char*) * argc);
    char* currentStr = (char*)(end + sizeof(char*)*argc);
    for (size_t i = 0; i < argc; i++) {
        // TODO: maybe deduplicate the strlen calls
        size_t len = strlen(argv[i]);
        memcpy(currentStr, argv[i], len);
        currentStr[len] = '\0';
        currentStr += (len + 1);
    }

    // add the kSegDescs
    args->kernelSegments = (KernelSegmentLoc*)(args + args->totalSize);
    args->kernelSegmentsCount = kSegs.len;
    for (int i = 0; i < args->kernelSegmentsCount; i++) {
        args->kernelSegments[i] = kSegs[i];
    }
    // add kargs itself
    args->kernelSegments[args->kernelSegmentsCount].len = args->totalSize;
    args->kernelSegments[args->kernelSegmentsCount].physLoc = (uint64_t)args;

    args->kernelSegmentsCount += 1;

    // do the memory map things
    // get the start ptr by end of kargs - mmap size
    MemoryDescriptor* mmapStart = (MemoryDescriptor*)((size_t)args + args->totalSize - mmapSize - kSegSize);
    int acc_len = 0;
    // convert over to our nice mmaps
    for (int i = 0; i < origInfo.fullSize; i += origInfo.descriptorSize) {
        efi_memory_descriptor_t* descriptor = (efi_memory_descriptor_t*)(mmapOrig + i);
        if (descriptor->Type < 0 || descriptor->Type >= MaxMemoryType) continue;
        mmapStart[acc_len].type = static_cast<MemoryType>(descriptor->Type);
        mmapStart[acc_len].physStart = descriptor->PhysicalStart;
        mmapStart[acc_len].nrPages = descriptor->NumberOfPages;
        mmapStart[acc_len].attr = descriptor->Attribute;
        acc_len += 1;
    }

    // set len
    args->memDescCount = acc_len;
    // update the size
    args->totalSize -= (mmapSize - acc_len*sizeof(MemoryDescriptor));
    args->kernelSegments[args->kernelSegmentsCount - 1].len = args->totalSize;
    // set the ptr
    args->memDesc = mmapStart;



    // escape from the bootservices!!
    if (exit_bs() != 0) {
        printf("Failed to exit bootservices.\n");
        while(1);
    }

    // interrupts are a No Thank You
    __asm__("cli");

    // while(1);
    // okay we should be good to go
    entry(args, kEntry);
    while(1);
}
