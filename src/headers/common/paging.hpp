#ifndef KERNEL_MEMORY_PAGING_HPP
#define KERNEL_MEMORY_PAGING_HPP

#include <stdint.h>

namespace memory {
struct PageDirectoryEntry {
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
    // free space
    bool availableBit : 1;
    // page size - are we mapping to a 4MiB page instead of a directory of 4KiB ones? (no)
    bool bigPage : 1 = false;
    // free space 2
    uint8_t available : 4;
    // the actual address
    uint64_t addr: 40;
    // more free space
    uint16_t available_2 : 11;
    // is execution disabled on this page?
    bool disableExecute : 1;
} __attribute__((__packed__));

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
} __attribute__((__packed__));
}

#endif // !KERNEL_MEMORY_PAGING_HPP
