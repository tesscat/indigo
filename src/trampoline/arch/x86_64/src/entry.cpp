#include "defs.hpp"
#include "loader/kernel_args.hpp"
#include "loader/memory_descriptor.hpp"
#include "paging.hpp"
#include <cstdint>

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;
unsigned int* fb;
uint64_t kEntry;

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
    entry->writeThrough = false; // TODO: check this and below
    entry->cacheDisabled = false;
    entry->bigPage = false;
    entry->disableExecute = true;
    entry->availableBit_orGlobal = false;
}

void setFullPermsAndPresent(memory::PageEntryBase* entry) {
    entry->present = true;
    entry->rw = true;
    entry->allAccessible = true; // TODO: check
    entry->writeThrough = false; // TODO: check this and below
    entry->cacheDisabled = false;
    entry->bigPage = false;
    entry->disableExecute = false;
    entry->availableBit_orGlobal = false;
}

// void setRecursiveMapping(memory::PageEntryBase* table) {
//     setPageMapPerms(&table[511]);
//     table[511].setAddr((uint64_t) table);
// }

extern "C" void enablePaging(uint64_t addr);

extern "C" void returnTo() {
    void (* __attribute__((sysv_abi)) kEntryFn)(KernelArgs*);
    kEntryFn = (void (* __attribute__((sysv_abi)))(KernelArgs*))kEntry;

    kEntryFn(kargs);

    loop_forever;
}

extern "C" void _start(KernelArgs* args, uint64_t kEntryVAddr) {
    kargs = args;
    kEntry = kEntryVAddr;
    fb = kargs->framebuffer;
    // set up a nice pagemap
    // identity map wherever we are, and map 48MiB to idk 50MiB to KERNEL_OFFSET + 48MiB
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

    // incrementable variable to track where we should allocate things
    uint64_t nextFreeMem = physAddr;

    // lovely, we have a physAddr.
    // each one of these represents 512gb
    memory::PageMapL4* pageMapTL = (memory::PageMapL4*) nextFreeMem;
    nextFreeMem += 512*sizeof(memory::PageMapL4);
    // the top one is unusable, since we need to recursively map it.
    // setRecursiveMapping(pageMapTL);

    // we are hardly using any of this stuff lol
    // just bottom entry
    for(int i = 1; i < 512; i++) {
        // pageMapTL[i].present = false;
        pageMapTL[i].clear();
    }


    // identity map 0 -> 32mb since that's UEFI stuff
    // make a PDP table
    memory::PageDirPointer* pageMapLowPDPD = (memory::PageDirPointer*) nextFreeMem;
    nextFreeMem += 512*sizeof(memory::PageDirPointer);

    // recursive mapping
    // setRecursiveMapping(pageMapLowPDPD);
    
    // bind the table
    // it resolves to the most restrictive perms, so as long as we set them on the lower levels we can use full perms higher up
    setFullPermsAndPresent(&pageMapTL[0]);
    pageMapTL[0].setAddr((uint64_t)pageMapLowPDPD);
    

    // again we are hardly using any of this
    for(int i = 1; i < 512; i++) {
        // pageMapLowPDPD[i].present = false;
        pageMapLowPDPD[i].clear();
    }

    // we're handling the map for a gigabyte. no likey. move down.
    // each one of these is two megs
    memory::PageDirEntry* pageDir = (memory::PageDirEntry*) nextFreeMem;
    nextFreeMem += 512*sizeof(memory::PageDirEntry);

    // recursive mapping
    // setRecursiveMapping(pageDir);

    // identity map 0 -> 32mb -> tramp_end
    uint64_t tramp_size = (uint64_t)&_tramp_end - (uint64_t)&_tramp_start;
    // round to the 2mbs
    int extra = (tramp_size / (1024*2048)) + 1;
    for (int i = 0; i < 16+extra; i++) {
        // it's UEFI stuff. i don't know WHAT it's doing
        setFullPermsAndPresent(&pageDir[i]);
        pageDir[i].availableBit_orGlobal = false;
        pageDir[i].bigPage = true;
        pageDir[i].setAddr((1024*2048) * i);
    }
    
    // bind the table
    setFullPermsAndPresent(&pageMapLowPDPD[0]);
    pageMapLowPDPD[0].setAddr((uint64_t)pageDir);


    // we wanna map 48MB to KERNEL_OFFSET + 48MB
    // which is in the highest 2GiB, so its obvs the last one
    memory::PageDirPointer* kernelPDP = (memory::PageDirPointer*) nextFreeMem;
    nextFreeMem += 512*sizeof(memory::PageDirPointer);


    // this does mean if we try to access high kernel memory we will simply Run Out of RAM so we gotta be careful
    // user processes _should_ permission error tho so we good on that
    // map the highest 2GiB
    // empty the rest of the map first
    for (int i = 0; i < 510; i++) {
        kernelPDP[i].clear();
    }
    for (int i = 510; i < 512; i++) {
        // kernel perms
        kernelPDP[i].present = true;
        kernelPDP[i].rw = true;
        kernelPDP[i].allAccessible = false;
        kernelPDP[i].writeThrough = true;
        kernelPDP[i].cacheDisabled = false;
        kernelPDP[i].bigPage = true;
        // don't set global since this table will be overwritten
        kernelPDP[i].availableBit_orGlobal = false;
        // we mapping the GiBs, remember
        kernelPDP[i].setAddr((i-510) * (1024 * 1024 * 1024));
        kernelPDP[i].disableExecute = false;
    }

    setFullPermsAndPresent(&pageMapTL[511]);
    pageMapTL[511].setAddr((uint64_t) kernelPDP);

    // we also need to ID-map the framebuffer
    // it's quite likely a few MiB in size, so we'll use that.
    //
    // This implementation will NOT work if it lies on a GiB boundary. we should probably do smth about that
    // TODO: see above
    //
    // which PML4 entry is it?
    uint64_t fb_pml4_idx = PML4_IDXOF((uint64_t)kargs->framebuffer);
    // we've only allocated 0 so far
    memory::PageDirPointer* gopPDPD;
    if (fb_pml4_idx == 0) {
        gopPDPD = pageMapLowPDPD;
    } else {
        // there is approximately No Way it's in kernel space
        gopPDPD = (memory::PageDirPointer*) nextFreeMem;
        nextFreeMem += 512*sizeof(memory::PageDirPointer);
        // setRecursiveMapping(gopPDPD);

        setFullPermsAndPresent(&pageMapTL[fb_pml4_idx]);
        pageMapTL[fb_pml4_idx].setAddr((uint64_t) gopPDPD);

        for (int i = 0; i < 512; i++) {
            // gopPDPD[i].present = false;
            gopPDPD[i].clear();
        }
    }
    // okie PDPD is sorted
    // now which GB is it in?
    uint64_t fb_pdp_idx = PDP_IDXOF((uint64_t)kargs->framebuffer);
    memory::PageDirEntry* gopPDE;
    if (gopPDPD[fb_pdp_idx].present) {
        gopPDE = (memory::PageDirEntry*)(gopPDPD[fb_pdp_idx].addr << 12);
    } else {
        gopPDE = (memory::PageDirPointer*) nextFreeMem;
        nextFreeMem += 512*sizeof(memory::PageDirEntry);
        // setRecursiveMapping(gopPDE);

        setFullPermsAndPresent(&gopPDPD[fb_pdp_idx]);
        gopPDPD[fb_pdp_idx].setAddr((uint64_t) gopPDE);

        for (int i = 0; i < 512; i++) {
            // gopPDE[i].present = false;
            gopPDE[i].clear();
        }
    }
    // okay we have the PDE. finally.

    // and what's the PDE idx of fb?
    uint64_t fb_pde_idx = PDE_IDXOF((uint64_t)kargs->framebuffer);
    // don't forget the thing will span multiple whatsits
    uint64_t fb_pde_idx_end = PDE_IDXOF(((uint64_t) kargs->framebuffer + (kargs->fbPitch * kargs->fbHeight * sizeof(unsigned int))));

    uint64_t addrHigh = ((1 << 30) * fb_pdp_idx) | (((uint64_t)1 << 39) * fb_pml4_idx);

    for (uint64_t i = fb_pde_idx; i < fb_pde_idx_end + 1; i++) {
        setFullPermsAndPresent(&gopPDE[i]);
        gopPDE[i].bigPage = true;
        gopPDE[i].setAddr(((1 << 21) * i) | addrHigh);
    }

    // update the values of kernelArgs since theyre remapped
    kargs->memDesc = (MemoryDescriptor*)((uint64_t)kargs->memDesc + KERNEL_OFFSET);
    kargs->kernelSegments = (KernelSegmentLoc*)((uint64_t)kargs->kernelSegments + KERNEL_OFFSET);
    // update all the char*s
    for(int i = 0; i < kargs->argc; i++) {
        kargs->argv[i] = (char*)((uint64_t)kargs->argv[i] + KERNEL_OFFSET);
    }
    // then the array ptr itself
    kargs->argv = (char**)((uint64_t)kargs->argv + KERNEL_OFFSET);
    
    // update kargs itself, since we've remapped it like 0xe... higher
    kargs = (KernelArgs*)((uint64_t)kargs + KERNEL_OFFSET);

    // okie the page tables are all set up
    // time for scary, we need to mov to CR3 the physical address
    enablePaging((uint64_t) pageMapTL);


    // this should unreachable.

    loop_forever;
}
