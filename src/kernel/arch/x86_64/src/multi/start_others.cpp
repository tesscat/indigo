#include "apic/lapic.hpp"
#include "inator/inator.hpp"
#include "memory/heap.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include "multi/cpu.hpp"
#include "sync/spinlock.hpp"
#include "util/util.hpp"
#include <cpuid.h>
#include <libmem/mem.hpp>

namespace multi {

// Structures the other CPU accesses to read/write it's own state
sync::Spinlock startupLock;
volatile uint64_t cpuIdx;

extern "C" uint64_t other_stack_top = 0;
extern "C" void trampoline_start();

void setupTrampolineCode() {
    // copy it to some low address, probably 0x8000
    memory::kernelIdentityMap4KiBBlock(0x8000);
    memory::markBlockAsUsed(0x8000, 4*KiB);
    // the UEFI loader should have done this already
    *(uint64_t*)(0x8f00) = (uint64_t)memory::kpgtable;
}

int startOthers() {
    setupTrampolineCode();
    // get my own lapic ID so i don't start myself
    unsigned int ebx, eax, unused;
    __cpuid(0x1, eax, ebx, unused, unused);
    ebx = ebx >> 24;

    startupLock.init();
    
    // send an INIT IPI to all APICs apart from my own
    *apic::lapic::errorStatus = 0;
    *apic::lapic::icr0 = 0b11001100010100000000;
    while (*apic::lapic::icr0 & (1<<12)) {__asm__ volatile ("pause" : : : "memory");}
    // // de-assert it
    *apic::lapic::errorStatus = 0;
    *apic::lapic::icr0 = 0b11001000010100000000;
    while (*apic::lapic::icr0 & (1<<12)) {__asm__ volatile ("pause" : : : "memory");}
    apic::apicSleep(10);

    for (uint64_t i = 0; i < nCpus; i++) {
        if (cpus[i].apicId == ebx) {
            // set our own CPU id
            util::cpuSetMSR(MSR_FSBASE, (i & 0xffffffff), (i >> 32));
            continue;
        }
        // let it get its idx
        cpuIdx = i;
        // lock startup on new thread's behalf
        startupLock.lock();
        other_stack_top = (uint64_t)kmalloc(16*KiB) + (16*KiB);

        // send SIPI twice
        for (int j = 0; j < 2; j++) {
            *apic::lapic::errorStatus = 0;
            *apic::lapic::icr1 = cpus[i].apicId << 24;
            *apic::lapic::icr0 = (0b00000000011000000000 | 0x8);
            apic::apicSleep(1);
            while (*apic::lapic::icr0 & (1<<12)) {__asm__ volatile ("pause" : : : "memory");}
        }
        startupLock.await();
    }
    return 0;
}

// submit this to the graph
void submitSMP() {
    inator::Target t;
    t.name = String("smp");
    t.provides.Append(String("smp"));
    t.preference = 1;
    t.load = startOthers;

    inator::graph->addTarget(t);
}
}
