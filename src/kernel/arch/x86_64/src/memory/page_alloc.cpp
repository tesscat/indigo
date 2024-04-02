#include "memory/page_alloc.hpp"
#include "loader/kernel_args.hpp"
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

// TODO: add support for mixing 2MiB pages in

// Note for when I get around to writing the slab allocator since we
// don't ID-page the entire space:
// just swap allocate4kPage() in the below function for something that allocates + maps it
// which would be a getOrCreateSubdirInKernelSpace which uses the slab alloc instead of allocate4kPage
// and then very pointedly un-allocates them later (important!!)

// Gets or creates the directory pointed to by the page base entry
template<uint64_t (*allocator)(), uint64_t neg_offset>
PageEntryBase* getOrCreateSubdir(PageEntryBase* entry, uint64_t flags) {
    uint64_t addr = entry->expandAddr();
    // since .clear() zeroes it, we assume if addr = 0 then the entry doesn't point to anything valid
    if (addr == 0) {
        addr = allocator();
        entry->setAddr(addr - neg_offset);
        for (int i = 0; i < 512; i++) {
            ((PageEntryBase*)addr)[i].clear();
        }
    } else {
        addr += neg_offset;
    }
    *((uint64_t*)entry) |= flags;
    return (PageEntryBase*) addr;
}

template<uint64_t (*allocator)(), uint64_t neg_offset = 0>
PageDirEntry* locateOrAllocatePDE(PageMap map, uint64_t virtAddr, uint64_t createFlags) {
    // the highest entry is guaranteed to exist
    uint64_t pml4_idx = PML4_IDXOF(virtAddr);
    PageMapL4* l4 = &map[pml4_idx];

    PageDirPointer* l3_dir = getOrCreateSubdir<allocator, neg_offset>(l4, createFlags);
    uint64_t pml3_idx = PDP_IDXOF(virtAddr);
    PageDirPointer* l3 = &l3_dir[pml3_idx];

    PageDirEntry* l2_dir = getOrCreateSubdir<allocator, neg_offset>(l3, createFlags);
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
template<uint64_t (*allocator)(), uint64_t neg_offset = 0>
PageTableEntry* locateOrAllocatePTE(PageMap map, uint64_t virtAddr, uint64_t createFlags) {
    
    PageDirEntry* l2 = locateOrAllocatePDE<allocator, neg_offset>(map, virtAddr, createFlags);

    PageTableEntry* l1_dir = (PageTableEntry*) getOrCreateSubdir<allocator, neg_offset>(l2, createFlags);
    PageTableEntry* l1 = &l1_dir[PTE_IDXOF(virtAddr)];
    return l1;
}

template<uint64_t (*allocator)(), uint64_t neg_offset = 0>
PageDirEntry* map2MiB(PageMap map, uint64_t virtAddr, uint64_t physAddr, uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE | PAGE_BIG_PAGE) {
    // get the PDE
    PageDirEntry* pde = locateOrAllocatePDE<allocator, neg_offset>(map, virtAddr, flags & (~PAGE_BIG_PAGE));
    pde->clear();
    pde->setAddr(physAddr);
    *((uint64_t*)pde) |= flags;

    return pde;
}

template<uint64_t (*allocator)(), uint64_t neg_offset = 0>
PageTableEntry* map4KiB(PageMap map, uint64_t virtAddr, uint64_t physAddr, uint64_t flags = PAGE_PRESENT | PAGE_READ_WRITE) {
    // get the PTE
    PageTableEntry* pte = locateOrAllocatePTE<allocator, neg_offset>(map, virtAddr, flags);
    pte->clear();
    pte->setAddr(physAddr);
    *((uint64_t*)pte) |= flags;

    return pte;
}

uint8_t initial_paging_scratch[4*MiB];
uint8_t* kpage_map_scratch;
uint8_t* paging_scratch;
uint64_t used_blocks;

uint64_t onetime_allocator() {
    used_blocks++;
    return (uint64_t)(kpage_map_scratch) + (4*KiB)*(used_blocks-1);
}

extern "C" char _kernel_virt_start;
extern "C" char _kernel_virt_end;


void initPageAllocator() {
    // Okay turns out we need to make a kpgtable now, since the trampoline one 
    // will probably be overwritten, since we don't block it out in the phys_alloc
    // find a 4kb aligned addr in initial_paging_scratch
    PageMap map = (PageMapL4*)util::roundUpToPowerOfTwo((uint64_t)initial_paging_scratch, 4*KiB);
    kpage_map_scratch = (uint8_t*)map;
    // clear the map
    for (int i = 0; i < 512; i++) {
        map[i].clear();
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
        map2MiB<onetime_allocator, KERNEL_OFFSET>(map, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 2*MiB;
    }

    // map the remainder as 4KiB
    // < since on last iter they should be set to equal
    while (curr_start_addr < curr_end_addr) {
        map4KiB<onetime_allocator, KERNEL_OFFSET>(map, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 4*KiB;
    }

    // okay, now map the framebuffer
    curr_start_addr = (uint64_t)kargs->framebuffer;
    curr_end_addr = util::roundUpToPowerOfTwo(((uint64_t)kargs->framebuffer + (kargs->fbPitch * kargs->fbHeight)*sizeof(uint32_t)), 4*KiB);
    // do the same 2MiB/4KiB split
    while (curr_start_addr + 2*MiB <= curr_end_addr) {
        map2MiB<onetime_allocator, KERNEL_OFFSET>(map, curr_start_addr, curr_start_addr);
        curr_start_addr += 2*MiB;
    }
    while (curr_start_addr < curr_end_addr) {
        map4KiB<onetime_allocator, KERNEL_OFFSET>(map, curr_start_addr, curr_start_addr);
        curr_start_addr += 4*KiB;
    }

    // we should (??) be good
    // let-sa try it!
    lcr3((uint64_t)map - KERNEL_OFFSET);
    graphics::psf::print("\nWOOOOOO\n");
}
}
