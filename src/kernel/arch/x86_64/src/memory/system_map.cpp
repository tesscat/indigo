#include "graphics/psf.hpp"
#include "libstd/merge_sort.hpp"
#include "loader/memory_descriptor.hpp"
#include "memory/basic_heap.hpp"
#include <cstdint>
#include <memory/system_map.hpp>
#include <libstd/itoa.hpp>

namespace memory {

void printAsHex(int n, int l = 16) {
    char sp[16];
    int f = itoa(n, sp, 16);
    sp[f] = '\0';
    graphics::psf::print("0x");
    for (int j = 0; j < (l-f); j++) graphics::psf::print("0");
    graphics::psf::print(sp);
}

SystemMemMapEntry memmaps_full[0x400];

SystemMap::SystemMap(MemoryDescriptor* mDescs, size_t mDescsCount, KernelSegmentLoc* kLocs, size_t kLocsCount) {
    // sort mDescs
    // mergeSort(mDescs, 0, mDescsCount);
    // uint8_t heap_back[0x4000];
    // basic_heap::BasicHeap heap {heap_back, 0x4000};
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

    // copy it across and add in kernel segments
    int j = 0;
    int curr_idx = 0;
    while (j < mmaps_idx) {
        int found = 1;
        uint64_t curr_start = mmaps[j].start;
        uint64_t curr_len = mmaps[j].len;
        while (found != 0) {
            if (curr_len == 0) break;
            // are there any kernel segments in the current sector?
            uint64_t lowestPoss = -1;
            uint64_t idx;
            found = 0;
            for (size_t k = 0; k < kLocsCount; k++) {
                if (kLocs[k].physLoc >= curr_start && kLocs[k].physLoc <= curr_start + curr_len && kLocs[k].len != 0) {
                    // its in
                    if (kLocs[k].physLoc < lowestPoss || found == 0) {
                        found++;
                        lowestPoss = kLocs[k].physLoc;
                        idx = k;
                    }
                }
            }
            if (found != 0) {
                // ok there are `found` segments. split us on up.
                if (lowestPoss != mmaps[j].start) {
                    // add in a free bit beforehand
                    memmaps_full[curr_idx].start = curr_start;
                    memmaps_full[curr_idx].len = lowestPoss - curr_start;
                    memmaps_full[curr_idx].type = Free;
                    curr_idx++;
                }
                // add in the kernelly bit
                memmaps_full[curr_idx].start = lowestPoss;
                memmaps_full[curr_idx].len = kLocs[idx].len;
                memmaps_full[curr_idx].type = Kernel;
                curr_idx++;
                curr_start = lowestPoss + kLocs[idx].len;
                curr_len -= kLocs[idx].len;
                // mark it as done
                kLocs[idx].len = 0;
            } else {
                // add in whatever we have
                memmaps_full[curr_idx].start = curr_start;
                memmaps_full[curr_idx].len = curr_len;
                memmaps_full[curr_idx].type = Free;
                curr_idx++;
            }
        }
        j++;
    }

    memMap = memmaps_full;
    memMapLen = curr_idx;

// #ifdef DEBUG_OUTPUT

    uint64_t tUsable = 0;
    uint64_t tKernel = 0;

    graphics::psf::print("\n");
    for (int i =0; i < curr_idx; i++) {
        graphics::psf::print("\n");
        printAsHex(memMap[i].start);

        int end = memMap[i].start + memMap[i].len;
        graphics::psf::print(" | ");
        printAsHex(end);
        graphics::psf::print(" | ");
        graphics::psf::print(memMap[i].type == Free ? "Free" : "Kernel");
        tUsable += memMap[i].len;
        if (memMap[i].type == Kernel) tKernel += memMap[i].len;
    }


    graphics::psf::print("\nTotal usable: ");
    printAsHex(tUsable);
    graphics::psf::print("\nTotal used by kernel: ");
    printAsHex(tKernel);
// #endif
}

}
