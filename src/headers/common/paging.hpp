#ifndef KERNEL_MEMORY_PAGING_HPP
#define KERNEL_MEMORY_PAGING_HPP

#include <stdint.h>

namespace memory {

// The template for the higher-order entries
struct PageEntryBase {
    // is this present in physical memory (ie if not we raise a page fault)
    bool present : 1;
    // read-write
    bool rw : 1;
    // user-supervisor, 1 means accessible by all, 0 means only kernel. remember to set for all higher entries too
    bool allAccessible : 1;
    // is write-through caching enabled? if not, this will be write-back cached
    bool writeThrough : 1;
    // is cache disabled for this page?
    bool cacheDisabled : 1;
    // has this page been accessed since we last reset this bit?
    bool accessed : 1;
    // free space. this is Dirty if we're bigPaging
    bool availableBit_orDirty : 1;
    // page size - are we mapping to a 2MiB page instead of a directory of 4KiB ones? (no)
    // This is reserved (0) on PML4-s
    bool bigPage : 1 = false;
    // should we NOT invalidate the TLB after we MOV to cr3?
    // free space if not bigPage
    uint8_t availableBit_orGlobal : 1;
    // free space 2
    uint8_t available : 3;
    // the actual address
    uint64_t addr: 40;
    // more free space
    uint16_t available_2 : 11;
    // is execution disabled on this page?
    bool disableExecute : 1;

    inline void setAddr(uint64_t physaddr) {
        addr = (physaddr >> 12);
    }
    inline void clear() {
        *((uint64_t*)this) = 0;
    }
} __attribute__((__packed__));

// highest level, points to PageDirPointer[512], one of these is 512gb
using PageMapL4 = PageEntryBase;
// second level, points to PageDirEntry[512], one of these is 1gb
using PageDirPointer = PageEntryBase;
// third level, points to PageTableEntry[512], one of these is 2 MiB
using PageDirEntry = PageEntryBase;

// The lowest-level one, this points to the actual RAM
struct PageTableEntry {
    // is this present in physical memory (ie if not we raise a page fault)
    bool present : 1;
    // read-write
    bool rw : 1;
    // user-supervisor, 1 means accessible by all, 0 means only kernel. remember to set for all higher entries too
    bool allAccessible : 1;
    // is write-through caching enabled? if not, this will be write-back cached
    bool writeThrough : 1;
    // is cache disabled for this page?
    bool cacheDisabled : 1;
    // has this page been accessed since we last reset this bit?
    bool accessed : 1;
    // has this page been written to (since presumably we last reset this? TODO: check)
    bool dirty : 1;
    // something about caching - TODO
    bool pageAttributeTable : 1;
    // global, tells processor to not invalidate the TLB instruction for this page upon a MOV to CR3 (TODO: research TLBs and CRs)
    bool global : 1;
    // free space
    uint8_t available : 3;
    // the actual address
    uint64_t addr: 40;
    // more free space
    uint16_t available_2 : 7;
    // protection key - TODO: research this
    uint8_t protectionKey : 4;
    // is execution disabled on this page?
    bool disableExecute : 1;

    inline void setAddr(uint64_t physaddr) {
        addr = (physaddr >> 12);
    }
    inline void clear() {
        *((uint64_t*)this) = 0;
    }
} __attribute__((__packed__));

#define PML4_IDXOF(addr) (((addr) >> 39) & 511)
#define PDP_IDXOF(addr) (((addr) >> 30) & 511)
#define PDE_IDXOF(addr) (((addr) >> 21) & 511)
#define PTE_IDXOF(addr) (((addr) >> 12) & 511)

}

#endif // !KERNEL_MEMORY_PAGING_HPP