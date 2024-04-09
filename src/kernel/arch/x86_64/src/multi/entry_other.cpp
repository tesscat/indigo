#include "apic/gdt.hpp"
#include "graphics/psf.hpp"
#include "multi/cpu.hpp"
#include "sync/spinlock.hpp"
#include "util/util.hpp"
#include <multi/start_others.hpp>

namespace multi {
extern sync::Spinlock startupLock;
extern uint64_t cpuIdx;
}

extern "C" void entry_other() {
    // load proper GDT
    apic::loadGdt();
    // set our cpu idx
    __asm__ volatile ("movw %0, %%fs" : : "rim"(multi::cpuIdx));
    // properly paged not proper GDT'd loaded
    graphics::psf::print("hello from other id: ");
    util::printAsHex(multi::getCpuIdx());
    graphics::psf::print("\n");
    // tell BSP we are living
    multi::startupLock.release();
    loop_forever;
}
