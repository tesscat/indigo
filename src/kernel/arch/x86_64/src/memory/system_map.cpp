#include "graphics/psf.hpp"
#include "loader/memory_descriptor.hpp"
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

SystemMemMapEntry memMap[0x400];
uint64_t memMapLen;

extern "C" char _kernel_phys_start;
extern "C" char _kernel_phys_end;

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
    
    uint64_t kStart = (uint64_t) &_kernel_phys_start;
    uint64_t kEnd = (uint64_t) &_kernel_phys_end;
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

#ifdef DEBUG_OUTPUT

    uint64_t tUsable = 0;
    uint64_t tKernel = 0;

    graphics::psf::print("\nFull data:\n");
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
#endif
}

}
