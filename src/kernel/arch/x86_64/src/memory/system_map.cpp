#include "defs.hpp"
#include "loader/memory_descriptor.hpp"
#include <cstdint>
#include <memory/system_map.hpp>
#include <libstd/itoa.hpp>

namespace memory {

extern "C" char _kernel_virt_start;
extern "C" char _kernel_virt_end;

SystemMemMapEntry memMap[0x400];
uint64_t memMapLen;

void initSystemMap(MemoryDescriptor* mDescs, size_t mDescsCount) {
    SystemMemMapEntry mmaps[0x400];
    int mmaps_idx = 0;
    size_t i = 0;
    while (i < mDescsCount) {
        if (!isUsableMemory(mDescs[i].type)) {
            i++;
            continue;
        }
        // try and merge the indexes
        size_t j = i;
        while (j+1 < mDescsCount 
            && isUsableMemory(mDescs[j+1].type)
            && mDescs[j].physStart+(4096*mDescs[j].nrPages) == mDescs[j+1].physStart
        ) {j++;}

        // we can merge from i through j
        if (i == j) {
            // ok we cannot
            mmaps[mmaps_idx].type = Free;
            mmaps[mmaps_idx].start = mDescs[i].physStart;
            mmaps[mmaps_idx].len = mDescs[i].nrPages * 4096;
        } else {
            mmaps[mmaps_idx].type = Free;
            mmaps[mmaps_idx].start = mDescs[i].physStart;
            mmaps[mmaps_idx].len = mDescs[j].physStart - mDescs[i].physStart + 4096*mDescs[j].nrPages;
        }
        mmaps_idx += 1;

        i = j+1;
    }
    
    uint64_t kStart = (uint64_t) &_kernel_virt_start - KERNEL_OFFSET;
    uint64_t kEnd = (uint64_t) &_kernel_virt_end - KERNEL_OFFSET;
    // copy it across and add in kernel segments
    int j = 0;
    int curr_idx = 0;
    while (j < mmaps_idx) {
        uint64_t curr_start = mmaps[j].start;
        uint64_t curr_len = mmaps[j].len;
        uint64_t curr_end = curr_start + curr_len;

        // do we overlap at all?
        if (
            (curr_start <= kStart && curr_end >= kStart) ||
            (curr_start <= kEnd && curr_end >= kEnd)
        ) {
            // ah we do
            // split us up into a before, an after and a kernel seg
            if (curr_start < kStart) {
                // there's a before
                memMap[curr_idx].start = curr_start;
                memMap[curr_idx].len = kStart - curr_start;
                memMap[curr_idx].type = Free;
                curr_idx++;
            }
            // do the kernelly bit
            memMap[curr_idx].start = kStart;
            memMap[curr_idx].len = kEnd - kStart;
            memMap[curr_idx].type = Kernel;
            curr_idx++;
            // in case kernel spills out of one segment (it shouldn't really)
            while (kEnd > (mmaps[j].start + mmaps[j].len) && j < mmaps_idx) {
                j++;
            }
            if (j >= mmaps_idx) break;
            // do an after bit
            if (kEnd < curr_end) {
                memMap[curr_idx].start = kEnd;
                memMap[curr_idx].len = curr_end - kEnd;
                memMap[curr_idx].type = Free;
                curr_idx++;
            }
        } else {
            // we do not :)
            memMap[curr_idx].start = curr_start;
            memMap[curr_idx].len = curr_len;
            memMap[curr_idx].type = Free;
            curr_idx++;
        }
        j++;
    }

    memMapLen = curr_idx;
}

}
