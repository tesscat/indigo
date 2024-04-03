#include "memory/page_alloc.hpp"
#include "libmem/slab.hpp"
#include "loader/kernel_args.hpp"
#include "memory/phys_alloc.hpp"
#include "paging.hpp"
#include "util/util.hpp"

extern KernelArgs* kargs;

// little hack to get my hands on bBE
namespace graphics {
namespace screen {
extern uint64_t backBufferEnd;
}
}

namespace memory {

// Note for when I get around to writing the slab allocator since we
// don't ID-page the entire space:
// just swap allocate4kPage() in the below function for something that allocates + maps it
// which would be a getOrCreateSubdirInKernelSpace which uses the slab alloc instead of allocate4kPage
// and then very pointedly un-allocates them later (important!!)

// Gets or creates the directory pointed to by the page base entry
template<uint64_t (*allocator)()>
PageEntryBase* getOrCreateSubdir(PageEntryBase* entry, uint64_t flags) {
    uint64_t addr = entry->expandAddr();
    // since .clear() zeroes it, we assume if addr = 0 then the entry doesn't point to anything valid
    if (addr == 0) {
        addr = allocator();
        // in case we work with something in identity paged memory
        entry->setAddr(addr);
        for (int i = 0; i < 512; i++) {
            ((PageEntryBase*)addr)[i].clear();
        }
    }
    *((uint64_t*)entry) |= flags;
    return (PageEntryBase*) addr;
}

template<uint64_t (*allocator)()>
PageDirEntry* locateOrAllocatePDE(PageMap map, uint64_t virtAddr, uint64_t createFlags) {
    // the highest entry is guaranteed to exist
    uint64_t pml4_idx = PML4_IDXOF(virtAddr);
    PageMapL4* l4 = &map[pml4_idx];

    PageDirPointer* l3_dir = getOrCreateSubdir<allocator>(l4, createFlags);
    uint64_t pml3_idx = PDP_IDXOF(virtAddr);
    PageDirPointer* l3 = &l3_dir[pml3_idx];

    PageDirEntry* l2_dir = getOrCreateSubdir<allocator>(l3, createFlags);
    uint64_t pml2_idx = PDE_IDXOF(virtAddr);
    PageDirEntry* l2 = &l2_dir[pml2_idx];

    return l2;
}

// Returns the address of the PageTableEntry (lowest level)
// within a specified page map
// corresponding to a specific _virtual_ address
// Operates in 4k pages mode
// If the VA doesn't exist/the table isnt built right for it,
// allocates memory for the map using the physical memory allocator
// the memory allocated for the map is not mapped itself, we just hope since the kernel is id-mapped
// that it works fine
template<uint64_t (*allocator)()>
PageTableEntry* locateOrAllocatePTE(PageMap map, uint64_t virtAddr, uint64_t createFlags) {
    
    PageDirEntry* l2 = locateOrAllocatePDE<allocator>(map, virtAddr, createFlags);

    PageTableEntry* l1_dir = (PageTableEntry*) getOrCreateSubdir<allocator>(l2, createFlags);
    PageTableEntry* l1 = &l1_dir[PTE_IDXOF(virtAddr)];
    return l1;
}

template<uint64_t (*allocator)()>
PageDirEntry* map2MiB(PageMap map, uint64_t virtAddr, uint64_t physAddr, uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_BIG_PAGE) {
    // get the PDE
    PageDirEntry* pde = locateOrAllocatePDE<allocator>(map, virtAddr, flags & (~PAGE_BIG_PAGE));
    pde->clear();
    pde->setAddr(physAddr);
    *((uint64_t*)pde) |= flags;

    return pde;
}

template<uint64_t (*allocator)()>
PageTableEntry* map4KiB(PageMap map, uint64_t virtAddr, uint64_t physAddr, uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE) {
    // get the PTE
    PageTableEntry* pte = locateOrAllocatePTE<allocator>(map, virtAddr, flags);
    pte->clear();
    pte->setAddr(physAddr);
    *((uint64_t*)pte) |= flags;

    return pte;
}

uint8_t* initial_paging_scratch;
uint8_t* kpage_map_scratch;
uint64_t used_blocks;
PageMapL4* kpgtable;

libmem::SlabAllocator<PageTable> slab;
// // temporary space we can map any page table sized thing
// // in main memory to, to edit it
// // an array of pointers to 4KiB blocks that we have the PTEs for
// PageTable** tempMappings;
// // and the accompanying PTEs
// PageTableEntry* tempMappingEntries;
// // and some scratch space to set it up
// uint8_t initial_temp_scratch[2*KiB];

uint64_t onetime_allocator() {
    used_blocks++;
    return (uint64_t)(kpage_map_scratch) + (4*KiB)*(used_blocks-1);
}

extern "C" char _kernel_virt_start;
extern "C" char _kernel_virt_end;


void initPageAllocator() {
    // we need to pick an address for the initial paging scratch
    initial_paging_scratch = (uint8_t*) allocate2mPage();
    // Okay turns out we need to make a kpgtable now, since the trampoline one 
    // will probably be overwritten, since we don't block it out in the phys_alloc
    // find a 4kb aligned addr in initial_paging_scratch
    kpgtable = (PageMapL4*)initial_paging_scratch;
    kpage_map_scratch = (uint8_t*)kpgtable;
    // clear the map
    for (int i = 0; i < 512; i++) {
        kpgtable[i].clear();
    }

    uint64_t curr_start_addr = (uint64_t)&_kernel_virt_start;
    // the first addr not used by the kernel or kargs
    // since the backbuffer needs to be mapped we have to cope with that one too
    uint64_t curr_end_addr = util::roundUpToPowerOfTwo(graphics::screen::backBufferEnd, 4*KiB);

    // set up allocator for the locateOrAllocates
    used_blocks = 1;

    
    // try and first map 2MiB blocks
    // TODO: should this be <= or <
    //
    while (curr_start_addr + 2*MiB <= curr_end_addr) {
        map2MiB<onetime_allocator>(kpgtable, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 2*MiB;
    }

    // map the remainder as 4KiB
    // < since on last iter they should be set to equal
    while (curr_start_addr < curr_end_addr) {
        map4KiB<onetime_allocator>(kpgtable, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 4*KiB;
    }

    // okay, now map the framebuffer
    curr_start_addr = (uint64_t)kargs->framebuffer;
    curr_end_addr = util::roundUpToPowerOfTwo(((uint64_t)kargs->framebuffer + (kargs->fbPitch * kargs->fbHeight)*sizeof(uint32_t)), 4*KiB);
    // do the same 2MiB/4KiB split
    while (curr_start_addr + 2*MiB <= curr_end_addr) {
        map2MiB<onetime_allocator>(kpgtable, curr_start_addr, curr_start_addr);
        curr_start_addr += 2*MiB;
    }
    while (curr_start_addr < curr_end_addr) {
        map4KiB<onetime_allocator>(kpgtable, curr_start_addr, curr_start_addr);
        curr_start_addr += 4*KiB;
    }
    
    // identity map the chosen scratch buffer
    map2MiB<onetime_allocator>(kpgtable, (uint64_t)initial_paging_scratch, (uint64_t)initial_paging_scratch);

    // we must never again use onetime_allocator
    // the tables we just allocated won't ever be freed
    // which is chill, they're kernel mappings
    // if the _kernel_ needs to temporatily map a thing, we have a slab alloc here for that
    kpage_map_scratch += sizeof(PageTable) * used_blocks;
    // the elements is a bit of a guessing game
    // we can fit 512 PageTables in the 2MiB block, i doubt we're going to use more than 50 to map the kernel
    // and everything else, and we also need space for the bitmaps to go,
    // so I guess we choose 300?
    // TODO: actually solve this problem
    slab = libmem::SlabAllocator<PageTable>((PageTable*)kpage_map_scratch, 300);

    // we should (??) be good
    // let-sa try it!
    lcr3((uint64_t)kpgtable);


    graphics::psf::print("\nWOOOOOO\n");

}

uint64_t slab_allocator() {
    graphics::psf::print("SA hit\n");
    uint64_t addr = (uint64_t)slab.Alloc();
    if (addr == 0)
        panic("Kernel paging slab out of memory");
    return addr;
}

// Attempts to identity-map the 4KiB of memory starting at round-down physAddr
void kernelMap4KiBBlock(uint64_t physAddr) {
    // TODO: TLB shootdown??
    // flush any previous cache
    __asm__ volatile ("invlpg %0" : : "rm"(physAddr));
    // just map it girl
    map4KiB<slab_allocator>(kpgtable, (uint64_t)physAddr, (uint64_t)physAddr);
    lcr3((uint64_t)kpgtable);
}
// Attempts to identity-map the 2MiB of memory starting at round-down physAddr
void kernelMap2MiBBlock(uint64_t physAddr) {
    // TODO: TLB shootdown??
    // flush any previous cache
    __asm__ volatile ("invlpg %0" : : "rm"(physAddr));
    // just map it girl
    map2MiB<slab_allocator>(kpgtable, (uint64_t)physAddr, (uint64_t)physAddr);
}

}
