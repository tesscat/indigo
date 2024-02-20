#include "loader/kernel_args.hpp"
#include "loader/memory_descriptor.hpp"

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;

extern char _tramp_start;
extern char _tramp_end;

inline bool doesEncompass(uint64_t start, uint64_t len, uint64_t val) {
    return start <= val && start+len >= val;
}

inline bool doesEncompassRange(uint64_t start, uint64_t end, uint64_t val) {
    return start <= val && end >= val;
}

bool isUsedByCode(uint64_t addr) {
    // for every kernel segment
    for (int i = 0; i < kargs->kernelSegmentsCount; i++) {
        // does it encompass the start or the start + 4MiB?
        if (doesEncompass(kargs->kernelSegments[i].physLoc, kargs->kernelSegments[i].len, addr) ||
            doesEncompass(kargs->kernelSegments[i].physLoc, kargs->kernelSegments[i].len, addr + (1024*4096)) ||
            // do we encompass the kernel segment?
            doesEncompass(addr, (1024*4096), kargs->kernelSegments[i].physLoc)
        ) {
            return true;
        }
    }
    // do _we_ use it?
    if (doesEncompassRange((uint64_t)&_tramp_start, (uint64_t)&_tramp_end, addr) ||
        doesEncompassRange((uint64_t)&_tramp_start, (uint64_t)&_tramp_end, addr + (1024*4096)) ||
        doesEncompass(addr, (1024*4096), (uint64_t)&_tramp_start)
        ) {
        return true;
    }
    return false;
}

extern "C" void _start(KernelArgs* args) {
    kargs = args;
    // set up a nice pagemap
    // identity map wherever we are, and map 48MiB to idk 50MiB to 0xe00000000000 + 48MiB
    // and use the recursive mapping trick
    // 
    // find a segment of suitable size
    volatile uint64_t physAddr = 0;
    int i;
    for (i = 0; i < args->memDescCount; i++) {
        // find the page-aligned addr
        uint64_t addr = args->memDesc[i].physStart;
        {
            uint64_t remnant = addr%4096;
            if (remnant != 0) {
                addr += (4096-remnant);
            }
        }
        if (
            // can we use it
            isUsableMemory(args->memDesc[i].type) &&
            // is it big enough (at least 4MiB)
            args->memDesc[i].nrPages > 1024 &&
            // this bit is complicated. is it a kernel thingy?
            !isUsedByCode(addr)
        ) {
            // woop woop
            physAddr = addr;
            break;
        }
    }
    if (i == args->memDescCount) {
        // failed to find one. this is Rather Unusual
        // paint a pixel white and freeze i guess
        args->framebuffer[0] = 0xffffffff;
        loop_forever;
    }
    loop_forever;
}
