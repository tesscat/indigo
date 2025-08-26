#include "apic/gdt.hpp"
#include "apic/lapic.hpp"
#include "logs/logs.hpp"
#include "multi/cpu.hpp"
#include "sync/spinlock.hpp"
#include "util/util.hpp"
#include <apic/idt.hpp>
#include <multi/start_others.hpp>

namespace multi {
    extern sync::Spinlock startupLock;
}

extern "C" void entry_other() {
    __asm__ volatile("cli");

    // early init code
    // load proper GDT
    apic::loadGdt();
    // idt
    apic::enableIdt();
    // lapic
    apic::enableLapic();
    // set our CPU idx
    uint16_t apicId = (*apic::lapic::apicId >> 24);
    // some dodgy looping
    uint64_t i = 0;
    for (; i < multi::nCpus; i++) {
        if (multi::cpus[i].apicId == apicId) {
            util::cpuSetMSR(MSR_FSBASE, (i & 0xffffffff), (i >> 32));
            break;
        }
    }
    if (i == multi::nCpus) {
        panic("Unable to find APIC id");
    }
    // tell BSP we are living
    multi::startupLock.release();
    // After this we need to use locks properly
    logs::info << "hello from other id: " << multi::getCpuIdx() << '\n';

    loop_forever;
}
