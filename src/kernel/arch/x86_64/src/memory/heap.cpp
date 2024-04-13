#include "defs.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include "sync/spinlock.hpp"
#include "util/util.hpp"
#include <stdint.h>
#include <libmem/mem.hpp>

namespace memory {

struct MemHeader {
    bool used;
    MemHeader* prev;
    MemHeader* next;
    uint64_t size;
};

uint64_t physBase;
uint64_t len;
MemHeader* heap_start;
MemHeader* first_hole;

sync::Spinlock heapLock;


void initHeap() {
    heapLock.init();
    heapLock.release();
    // find some memory
    // allocate in 2MiB blocks so we need to mess the map up less
    physBase = allocate2mPage();
    kernelMap2MiBBlock(HEAP_VIRTUAL_BASE, physBase);
    len = 2*MiB;
    // chuck a header at the start
    heap_start = (MemHeader*) HEAP_VIRTUAL_BASE;
    heap_start->used = false;
    heap_start->prev = nullptr;
    heap_start->next = nullptr;
    heap_start->size = 2*MiB - sizeof(MemHeader);

    first_hole = heap_start;
}

void* Allocate(uint64_t size, MemHeader* search_from);

void* TryExpandAndReAllocate(uint64_t size, MemHeader* last) {
    // how many 2MiB blocks to map?
    uint64_t blocks = (size + (2*MiB-1))/(2*MiB);
    uint64_t oldTop = HEAP_VIRTUAL_BASE + len;
    for (uint64_t i = 0; i < blocks; i++) {
        uint64_t newBase = allocate2mPage();
        if (newBase == 0) panic("kernel alloc out of memory :(");
        kernelMap2MiBBlock(HEAP_VIRTUAL_BASE + len, newBase);
        len += 2*MiB;
    }
    if (last->used) {
        MemHeader* newLast = (MemHeader*)oldTop;
        newLast->used = false;
        newLast->prev = last;
        newLast->next = nullptr;
        newLast->size = blocks*2*MiB - sizeof(MemHeader);
        last->next = newLast;
        last = newLast;
    } else {
        last->size += blocks*2*MiB;
    }

    return Allocate(size, last);
}

void* Allocate(uint64_t size, MemHeader* search_from = first_hole) {
    // Find the first one that's enough
    uint64_t acc_size = size + sizeof(MemHeader);
    MemHeader* curr = search_from;
    MemHeader* last = curr;
    while (curr != nullptr && (curr->used || (curr->size < acc_size && curr->size != size))) {
        last = curr;
        curr = curr->next;
    }
    if (curr == nullptr) {
        return TryExpandAndReAllocate(size, last);
    }
    // we have one!
    // is it a replace or is it a split?
    if (curr->size == size) {
        // replace
        curr->used = true;
        return curr + sizeof(MemHeader);
    } else {
        // split time
        // make the new hole
        MemHeader* new_hole = (MemHeader*)((uint64_t)curr + acc_size);
        new_hole->used = false;
        new_hole->prev = curr;
        new_hole->next = curr->next;
        new_hole->size = curr->size - acc_size;
        // update this one
        curr->used = true;
        curr->next = new_hole;
        curr->size = size;
        return curr + sizeof(MemHeader);
    }
}

void TryMergeWithSuccessor(MemHeader* mh) {
    // assume succ is not null
    if (mh->next->used) return;
    // go for merge!
    mh->size += mh->next->size + sizeof(MemHeader);
    if (mh->next->next)
        mh->next->next->prev = mh;
    mh->next = mh->next->next;
}

}
void kfree(const void* addr) {
    memory::heapLock.lock();
    using namespace memory;
    // addr better be a MemHeader cuz imma cry if its not
    MemHeader* mh = (MemHeader*) addr - sizeof(MemHeader);
    if (!mh->used) return;
    // try merge forwards
    if (mh->next) {
        TryMergeWithSuccessor(mh);
    }
    // try merge backwards
    if (mh->prev && !mh->prev->used) {
        TryMergeWithSuccessor(mh->prev);
        mh = mh->prev;
    }

    if ((uint64_t)mh < (uint64_t)first_hole) {
        first_hole = mh;
    }
    memory::heapLock.release();
    return;
}

void* kmalloc(const uint64_t size) {
    using namespace memory;
    memory::heapLock.lock();
    void* a = Allocate(size, first_hole);
    memory::heapLock.release();
    return a;
}

void* krealloc(const void* addr, const uint64_t size) {
    using namespace memory;
    // TODO: check successor to see if we can flat expand
    MemHeader* mh = (MemHeader*) addr - sizeof(MemHeader);
    void* a2 = kmalloc(size);
    uint64_t s2 = size;
    if (mh->size < size) s2 = mh->size; 
    memcpy(a2, addr, s2);
    kfree(addr);
    return a2;
}

#include <stddef.h>

void* operator new(size_t size) {
    return kmalloc(size);
}

void* operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void *p) {
    kfree(p);
}

void operator delete[](void *p) {
    kfree(p);
}
