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
    *((uint64_t*)pde) |= flags | PAGE_BIG_PAGE;

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

// uint64_t used_blocks;
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

// uint64_t slab_allocator() {
//     used_blocks++;
//     return (uint64_t)(kpage_map_scratch) + (4*KiB)*(used_blocks-1);
// }

uint64_t hits = 0;
uint64_t slab_allocator() {
    hits++;
    uint64_t addr = (uint64_t)slab.Alloc();
    if (addr == 0)
        panic("Kernel paging slab out of memory");
    return addr;
}

extern "C" char _kernel_virt_start;
extern "C" char _kernel_virt_end;


void initPageAllocator() {
    hits = 0;
    // we need to pick an address for the initial paging scratch
    uint8_t* initial_paging_scratch = (uint8_t*) allocate2mPage();
    allocate2mPage();
    // give this to the slab alloc
    // 511 is a bit of a guess, ideally we'd have the slab alloc solve for the maximum it can fit
    // TODO: have slab solve for how much to allocate for the bitmap
    slab = libmem::SlabAllocator<PageTable>((PageTable*)initial_paging_scratch, 511);
    // Okay turns out we need to make a kpgtable now, since the trampoline one 
    // will probably be overwritten, since we don't block it out in the phys_alloc
    // find a 4kb aligned addr in initial_paging_scratch
    kpgtable = (PageMapL4*)slab.Alloc();
    // clear the map
    for (int i = 0; i < 512; i++) {
        kpgtable[i].clear();
    }

    uint64_t curr_start_addr = (uint64_t)&_kernel_virt_start;
    // the first addr not used by the kernel or kargs
    // since the backbuffer needs to be mapped we have to cope with that one too
    uint64_t curr_end_addr = util::roundUpToPowerOfTwo(graphics::screen::backBufferEnd, 4*KiB);

    // set up allocator for the locateOrAllocates
    // used_blocks = 1;

    
    // try and first map 2MiB blocks
    // TODO: should this be <= or <
    //
    while (curr_start_addr + 2*MiB <= curr_end_addr) {
        map2MiB<slab_allocator>(kpgtable, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 2*MiB;
    }

    // map the remainder as 4KiB
    // < since on last iter they should be set to equal
    while (curr_start_addr < curr_end_addr) {
        map4KiB<slab_allocator>(kpgtable, curr_start_addr, curr_start_addr - KERNEL_OFFSET);
        curr_start_addr += 4*KiB;
    }

    // okay, now map the framebuffer
    curr_start_addr = (uint64_t)kargs->framebuffer;
    curr_end_addr = util::roundUpToPowerOfTwo(((uint64_t)kargs->framebuffer + (kargs->fbPitch * kargs->fbHeight)*sizeof(uint32_t)), 4*KiB);
    // do the same 2MiB/4KiB split
    while (curr_start_addr + 2*MiB <= curr_end_addr) {
        map2MiB<slab_allocator>(kpgtable, curr_start_addr, curr_start_addr);
        curr_start_addr += 2*MiB;
    }
    while (curr_start_addr < curr_end_addr) {
        map4KiB<slab_allocator>(kpgtable, curr_start_addr, curr_start_addr);
        curr_start_addr += 4*KiB;
    }
    
    // identity map the chosen scratch buffer
    map2MiB<slab_allocator>(kpgtable, (uint64_t)initial_paging_scratch, (uint64_t)initial_paging_scratch);

    // we should (??) be good
    // let-sa try it!
    lcr3((uint64_t)kpgtable);


    graphics::psf::print("\nWOOOOOO\n");

}


void kernelMap4KiBBlock(uint64_t virtAddr, uint64_t physAddr, uint64_t flags) {
    __asm__ volatile ("invlpg %0" : : "rm"(virtAddr));
    // just map it girl
    map4KiB<slab_allocator>(kpgtable, (uint64_t)virtAddr, (uint64_t)physAddr, flags);
    lcr3((uint64_t)kpgtable);
}

// TODO: do the funky SIMD on this
// TODO: set up proper exists/non-exists criteria
inline bool isTableEmpty(PageEntryBase* arr) {
    for (int i = 0; i < 511; i++) {
        if (*(uint64_t*)&arr[i] != 0) return false;
    }
    return true;
}


// Unmaps memory previously set by KM4KiBBlock
// this is a bit more complex, since we need to track the tables all the way down + check if they empty
void kernelUnmap4KiBBlock(uint64_t virtAddr) {
    PageMapL4* l4 = &kpgtable[PML4_IDXOF(virtAddr)];
    PageDirPointer* l3_arr = (PageDirPointer*) l4->expandAddr();
    if (l3_arr == nullptr) return;
    PageDirPointer* l3 = &l3_arr[PDP_IDXOF(virtAddr)];
    PageDirEntry* l2_arr = (PageDirEntry*) l3->expandAddr();
    if (l2_arr == nullptr) return;
    PageDirEntry* l2 = &l2_arr[PDE_IDXOF(virtAddr)];
    PageTableEntry* l1_arr = (PageTableEntry*) l2->expandAddr();
    if (l1_arr == nullptr) return;
    PageTableEntry* l1 = &l1_arr[PTE_IDXOF(virtAddr)];
    // clear l1
    l1->clear();
    // TODO: TLB shootdowns
    __asm__ volatile ("invlpg %0" : : "rm"(virtAddr));
    // is all of l1_arr free?
    if (!isTableEmpty((PageEntryBase*) l1_arr)) return;
    slab.Free((PageTable*) l1_arr);
    // unpoint l2
    l2->clear();
    // repeat for higher levels
    if (!isTableEmpty(l2_arr)) return;
    slab.Free((PageTable*)l2_arr);
    l3->clear();
    if(!isTableEmpty(l3_arr)) return;
    slab.Free((PageTable*)l3_arr);
    l4->clear();
    // l4 shouldn't ever empty so we don't bother checking
}

void kernelMap2MiBBlock(uint64_t virtAddr, uint64_t physAddr, uint64_t flags) {
    // TODO: TLB shootdown??
    // flush any previous cache
    __asm__ volatile ("invlpg %0" : : "rm"(virtAddr));
    // just map it girl
    map2MiB<slab_allocator>(kpgtable, (uint64_t)virtAddr, (uint64_t)physAddr, flags);
}

void kernelUnmap2MiBBlock(uint64_t virtAddr) {
    PageMapL4* l4 = &kpgtable[PML4_IDXOF(virtAddr)];
    PageDirPointer* l3_arr = (PageDirPointer*) l4->expandAddr();
    if (l3_arr == nullptr) return;
    PageDirPointer* l3 = &l3_arr[PDP_IDXOF(virtAddr)];
    PageDirEntry* l2_arr = (PageDirEntry*) l3->expandAddr();
    if (l2_arr == nullptr) return;
    PageDirEntry* l2 = &l2_arr[PDE_IDXOF(virtAddr)];
    // clear l1
    l2->clear();
    // TODO: TLB shootdowns
    __asm__ volatile ("invlpg %0" : : "rm"(virtAddr));
    // repeat for higher levels
    if (!isTableEmpty(l2_arr)) return;
    slab.Free((PageTable*)l2_arr);
    l3->clear();
    if(!isTableEmpty(l3_arr)) return;
    slab.Free((PageTable*)l3_arr);
    l4->clear();
    // l4 shouldn't ever empty so we don't bother checking
}

void kernelMapBlock(uint64_t base, uint64_t size_, uint64_t flags) {
    // round base down to 4KiB
    uint64_t baseDown = base & ~(4*KiB-1);
    // bump size
    int64_t size = size_ + base & (4*KiB - 1);
    // while baseDown isn't 2MiB aligned, map 4KiB-wise
    while ((baseDown & (2*MiB-1)) != 0) {
        kernelIdentityMap4KiBBlock(baseDown, flags);
        baseDown += 4*KiB;
        size -= 4*KiB;
    }
    // map it to 2MiB
    while (size >= 2*MiB) {
        kernelIdentityMap2MiBBlock(baseDown, flags);
        baseDown -= 2*MiB;
        size -= 2*MiB;
    }
    // map the remainder with 4KiB
    while (size > 0) {
        kernelIdentityMap4KiBBlock(baseDown, flags);
        baseDown += 4*KiB;
        size -= 4*KiB;
    }
}
void kernelUnmapBlock(uint64_t base, uint64_t size) {
    // round base down to 4KiB
    uint64_t baseDown = base & ~(4*KiB-1);
    // bump size
    size += base & (4*KiB - 1);
    // while baseDown isn't 2MiB aligned, map 4KiB-wise
    while ((baseDown & (2*MiB-1)) != 0) {
        kernelUnmap4KiBBlock(baseDown);
        baseDown += 4*KiB;
        size -= 4*KiB;
    }
    // map it to 2MiB
    while (size >= 2*MiB) {
        kernelUnmap2MiBBlock(baseDown);
        baseDown -= 2*MiB;
        size -= 2*MiB;
    }
    // map the remainder with 4KiB
    while (size > 0) {
        kernelUnmap4KiBBlock(baseDown);
        baseDown += 4*KiB;
        size -= 4*KiB;
    }
}
}
