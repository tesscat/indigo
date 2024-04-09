#include "apic/lapic.hpp"
#include "memory/heap.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include "multi/cpu.hpp"
#include "sync/spinlock.hpp"
#include "util/util.hpp"
#include <cpuid.h>
#include <libmem/mem.hpp>

namespace multi {

sync::Spinlock startupLock;

extern "C" uint64_t other_stack_top = 0;
extern "C" void trampoline_start();

void setupTrampolineCode() {
    // copy it to some low address, probably 0x8000
    memory::kernelIdentityMap4KiBBlock(0x8000);
    memory::markBlockAsUsed(0x8000, 4*KiB);
    // the UEFI loader should have done this already
    // memcpy((void*)0x8000, (void*)&trampoline_start, 3*KiB);
    *(uint64_t*)(0x8f00) = (uint64_t)memory::kpgtable;
}

void startOthers() {
    setupTrampolineCode();
    // get my own lapic ID so i don't start myself
    unsigned int ebx, eax, unused;
    __cpuid(0x1, eax, ebx, unused, unused);
    ebx = ebx >> 24;

    uint8_t* lapic_ptr = (uint8_t*)LAPIC_BASE;

    startupLock.init();

    for (uint64_t i = 0; i < nCpus; i++) {
        if (cpus[i].apicId == ebx) continue;
        // lock startup on new thread's behalf
        startupLock.lock();
        other_stack_top = (uint64_t)kmalloc(16*KiB);
        // send a init IPI
        *apic::lapic::errorStatus = 0;
        *apic::lapic::icr1 = cpus[i].apicId << 24;
        *apic::lapic::icr0 = 0b1100010100000000;
        // de-assert the init IPI
        *apic::lapic::errorStatus = 0;
        *apic::lapic::icr1 = cpus[i].apicId << 24;
        *apic::lapic::icr0 = 0b1000010100000000;
        // send SIPI twice
        for (int j = 0; j < 2; j++) {
            *apic::lapic::errorStatus = 0;
            *apic::lapic::icr1 = cpus[i].apicId << 24;
            *apic::lapic::icr0 = (0b00000000011000000000 | 0x8);
        }
        startupLock.await();
    }
}
}
