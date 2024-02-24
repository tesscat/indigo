#include "loader/kernel_args.hpp"
#include "loader/memory_descriptor.hpp"
#include "paging.hpp"
#include <cstdint>

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;

extern char _tramp_start;
extern char _tramp_end;

inline bool doesEncompass(uint64_t start, uint64_t len, uint64_t val) {
    return start <= val && start+len >= val;
}

inline bool doesEncompassRange(uint64_t start, uint64_t end, uint64_t val) {
    return start <= val && end >= val;
}

bool isUsedByCode(uint64_t addr) {
    // for every kernel segment
    for (int i = 0; i < kargs->kernelSegmentsCount; i++) {
        // does it encompass the start or the start + 4MiB?
        if (doesEncompass(kargs->kernelSegments[i].physLoc, kargs->kernelSegments[i].len, addr) ||
            doesEncompass(kargs->kernelSegments[i].physLoc, kargs->kernelSegments[i].len, addr + (1024*4096)) ||
            // do we encompass the kernel segment?
            doesEncompass(addr, (1024*4096), kargs->kernelSegments[i].physLoc)
        ) {
            return true;
        }
    }
    // do _we_ use it?
    if (doesEncompassRange((uint64_t)&_tramp_start, (uint64_t)&_tramp_end, addr) ||
        doesEncompassRange((uint64_t)&_tramp_start, (uint64_t)&_tramp_end, addr + (1024*4096)) ||
        doesEncompass(addr, (1024*4096), (uint64_t)&_tramp_start)
    ) {
        return true;
    }
    return false;
}


void setPageMapPerms(memory::PageEntryBase* entry) {
    entry->present = true;
    entry->rw = true;
    entry->allAccessible = false; // TODO: check
    entry->writeThrough = true; // TODO: check this and below
    entry->cacheDisabled = false;
    entry->bigPage = false;
    entry->disableExecute = true;
    entry->availableBit_orGlobal = false;
}

void setFullPerms(memory::PageEntryBase* entry) {
    entry->present = true;
    entry->rw = true;
    entry->allAccessible = true; // TODO: check
    entry->writeThrough = true; // TODO: check this and below
    entry->cacheDisabled = false;
    entry->bigPage = false;
    entry->disableExecute = false;
    entry->availableBit_orGlobal = false;
}

void setRecursiveMapping(memory::PageEntryBase* table) {
    setPageMapPerms(&table[511]);
    table[511].setAddr((uint64_t) table);
}

extern "C" void enablePaging(uint64_t addr);

extern "C" void returnTo() {
    // kargs -> frameBuffer is currently unmapped
    // TODO: need to fix paging to map this up
    loop_forever;
}

extern "C" void _start(KernelArgs* args) {
    kargs = args;
    // set up a nice pagemap
    // identity map wherever we are, and map 48MiB to idk 50MiB to 0xe00000000000 + 48MiB
    // and use the recursive mapping trick
    // 
    // find a segment of suitable size
    uint64_t physAddr = 0;
    int i;
    for (i = 0; i < args->memDescCount; i++) {
        // find the page-aligned addr
        uint64_t addr = args->memDesc[i].physStart;
        {
            uint64_t remnant = addr%4096;
            if (remnant != 0) {
                addr += (4096-remnant);
            }
        }
        if (
            // can we use it
            isUsableMemory(args->memDesc[i].type) &&
            // is it big enough (at least 4MiB)
            args->memDesc[i].nrPages > 1024 &&
            // this bit is complicated. is it a kernel thingy?
            !isUsedByCode(addr)
        ) {
            // woop woop
            physAddr = addr;
            break;
        }
    }
    if (i >= args->memDescCount) {
        // failed to find one. this is Rather Unusual
        // paint a pixel white and freeze i guess
        args->framebuffer[0] = 0xffffffff;
        loop_forever;
    }

    // lovely, we have a physAddr.
    // each one of these represents 512gb
    memory::PageMapL4* pageMapTL = (memory::PageMapL4*) physAddr;
    // the top one is unusable, since we need to recursively map it.
    setRecursiveMapping(pageMapTL);

    // we are hardly using any of this stuff lol
    // just top + bottom entry
    for(int i = 1; i < 511; i++) {
        pageMapTL[i].present = false;
    }


    // identity map 0 -> 32mb since that's UEFI stuff
    // make a PDP table
    memory::PageDirPointer* pageMapLowPDPD = (memory::PageDirPointer*) physAddr + 512*sizeof(memory::PageMapL4);
    
    // bind the table
    // it resolves to the most restrictive perms, so as long as we set them on the lower levels we can use full perms higher up
    setFullPerms(&pageMapTL[0]);
    pageMapTL[0].setAddr((uint64_t)pageMapLowPDPD);
    

    // recursive mapping
    setRecursiveMapping(pageMapLowPDPD);
    // again we are hardly using any of this
    for(int i = 1; i < 511; i++) {
        pageMapLowPDPD[i].present = false;
    }

    // we're handling the map for a gigabyte. no likey. move down.
    // each one of these is two megs
    memory::PageDirEntry* pageDir = (memory::PageDirEntry*) (uint64_t)pageMapLowPDPD + 512*sizeof(memory::PageDirPointer);
    setRecursiveMapping(pageDir);
    // identity map 0 -> 32mb -> tramp_end
    uint64_t tramp_size = (uint64_t)&_tramp_end - (uint64_t)&_tramp_start;
    // round to the 2mbs
    int extra = (tramp_size / (1024*2048)) + 1;
    for (int i = 0; i < 16+extra; i++) {
        // it's UEFI stuff. i don't know WHAT it's doing
        setFullPerms(&pageDir[i]);
        pageDir[i].availableBit_orGlobal = false;
        pageDir[i].bigPage = true;
        pageDir[i].setAddr((1024*2048) * i);
    }
    
    // bind the table
    setFullPerms(&pageMapLowPDPD[0]);
    pageMapLowPDPD[0].setAddr((uint64_t)pageDir);


    // we wanna map 48MB to 0xe00000000000 + 48MB
    // what index in the TL is 0xe... at? 
    // It's 229376 GiB, and each one is 512 GiB,
    // so it's at 448. which is coincidentally (0xe/0x10) through the table
    // (its not a coincidence)
    // we'll just map like 512GiB, nobody is ever going to want memory at like Over There
    // since we map the gigabyte, this does have the effect of binding physical 0 -> 48MiB to both virtual 0->48MiB as we did earlier and 0xe.. -> +48MiB but who cares
    memory::PageDirPointer* kernelPDP = (memory::PageDirPointer*) (uint64_t)pageDir + 512*sizeof(memory::PageDirEntry);
    setRecursiveMapping(kernelPDP);
    // this does mean if we try to access high kernel memory we will simply Run Out of RAM so we gotta be careful
    // user processes _should_ permission error tho so we good on that
    for (int i = 0; i < 511; i++) {
        // kernel perms
        kernelPDP[i].present = true;
        kernelPDP[i].rw = true;
        kernelPDP[i].allAccessible = false;
        kernelPDP[i].writeThrough = true;
        kernelPDP[i].cacheDisabled = false;
        kernelPDP[i].bigPage = true;
        // we can set global, since kernel is going to be mapped in the same place regardless of the user process
        kernelPDP[i].availableBit_orGlobal = true;
        // we mapping the GiBs, remember
        kernelPDP[i].setAddr(i * (1024 * 1024 * 1024));
        kernelPDP[i].disableExecute = false;
    }

    setFullPerms(&pageMapTL[448]);
    pageMapTL[448].setAddr((uint64_t) kernelPDP);

    // okie the page tables are all set up
    // time for scary, we need to mov to CR3 the physiacl address
    enablePaging((uint64_t) pageMapTL);

    // celebratory pixel
    kargs->framebuffer[0] = 0xff00ffff;

    loop_forever;
}
