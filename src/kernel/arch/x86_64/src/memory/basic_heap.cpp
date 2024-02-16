#include <memory/basic_heap.hpp>
#include <stdint.h>

namespace memory::basic_heap {
struct MemHeader {
    bool used;
    MemHeader* prev;
    MemHeader* next;
    size_t size;
};

BasicHeap::BasicHeap(void* base, size_t len_) : start{base}, len{len_} {
    // really, len should be bigger than like 18? but this is up to the user to fix
    // chuck a header at the start
    MemHeader* heap_start = (MemHeader*) base;
    heap_start->used = false;
    heap_start->prev = nullptr;
    heap_start->next = nullptr;
    heap_start->size = len - sizeof(MemHeader);
}

void* BasicHeap::Allocate(size_t size) {
    // Find the first one that's enough
    size_t acc_size = size + sizeof(MemHeader);
    MemHeader* curr = (MemHeader*)start;
    while ((curr->used || (curr->size < acc_size && curr->size != size)) && curr != nullptr) {
        curr = curr->next;
    }
    if (curr == nullptr) return nullptr;
    // we have one!
    // is it a replace or is it a split?
    if (curr->size == size) {
        // replace
        curr->used = true;
        return curr + sizeof(MemHeader);
    } else {
        // split time
        // make the new hole
        MemHeader* new_hole = curr + acc_size;
        new_hole->used = false;
        new_hole->prev = curr;
        new_hole->next = curr->next;
        new_hole->size = curr->size - acc_size;
        // update the next
        curr->next->prev = new_hole;
        // update this one
        curr->used = true;
        curr->next = new_hole;
        curr->size = size;
        return curr + sizeof(MemHeader);
    }
}

void BasicHeap::TryMergeWithSuccessor(MemHeader* mh) {
    // assume succ is not null
    if (mh->next->used) return;
    // go for merge!
    mh->size += mh->next->size + sizeof(MemHeader);
    mh->next->next->prev = mh;
    mh->next = mh->next->next;
}

int BasicHeap::Free(const void* addr) {
    // addr better be a MemHeader cuz imma cry if its not
    MemHeader* mh = (MemHeader*) addr - sizeof(MemHeader);
    if (!mh->used) return -1;
    // try merge forwards
    if (mh->next) {
        TryMergeWithSuccessor(mh);
    }
    // try merge backwards
    if (mh->prev && !mh->prev->used) {
        TryMergeWithSuccessor(mh->prev);
    }

    return 0;
}

}
