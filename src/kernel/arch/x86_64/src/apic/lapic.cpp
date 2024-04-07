#include "apic/lapic.hpp"
#include "acpi/acpi.hpp"
#include "apic/idt.hpp"
#include "graphics/psf.hpp"
#include "io/io.hpp"
#include "memory/page_alloc.hpp"
#include "memory/phys_alloc.hpp"
#include "paging.hpp"
#include "util/util.hpp"
#include <multi/cpu.hpp>
#include <memory/heap.hpp>
#include <libmem/mem.hpp>
#include <cpuid.h>

namespace apic {
uint64_t localApicPhysAddr;

// the LAPIC is at 0xfee00000 by _definition_

#define LAPIC_BASE 0xFEE00000
namespace lapic {
#define helper_uint32(name, offs) uint32_t volatile* name = reinterpret_cast<uint32_t*>(LAPIC_BASE + offs);
#define helper_uint32_const(name, offs) uint32_t const volatile* name = reinterpret_cast<uint32_t*>(LAPIC_BASE + offs);
#define helper_lvt(name, offs) LvtRegister volatile* name = reinterpret_cast<LvtRegister*>(LAPIC_BASE + offs);
helper_uint32(apicId, 0x20)
helper_uint32_const(apicVersion, 0x30)
helper_uint32(taskPriority, 0x80)
helper_uint32_const(arbitrationPriority, 0x90)
helper_uint32_const(processorPriority, 0xA0)
helper_uint32(eoi, 0xB0)
helper_uint32_const(remoteRead, 0xC0)
helper_uint32(logicalDestination, 0xD0)
helper_uint32(destinationFormat, 0xE0)
helper_uint32(spuriousInterruptVector, 0xF0)
// TODO: the In-Service register and Trigger Mode Register and Interrupt Request Register
helper_uint32(errorStatus, 0x280)
namespace lvt {
helper_lvt(correctedMachineCheck, 0x2F0)
helper_lvt(timer, 0x320)
helper_lvt(thermalSensor, 0x330)
helper_lvt(performanceMonitor, 0x340)
helper_lvt(lint0, 0x350)
helper_lvt(lint1, 0x360)
helper_lvt(error, 0x370)
}
helper_uint32(timerInitialCount, 0x380)
helper_uint32_const(timerCurrentCount, 0x390)
helper_uint32(timerDivideConfiguration, 0x3E0)
}

// LapicRegisters lapic(0x0);

__attribute__ ((interrupt)) void spurious(apic::InterruptFrame* frame) {
    loop_forever;
}
__attribute__ ((interrupt)) void timer(apic::InterruptFrame* frame) {
    graphics::psf::print("ee\n");
    // serve EOI
    *lapic::eoi = 0;
    return;
    // loop_forever;
}

uint32_t ticksPerSecond;

void initLapic() {
    // find the LAPIC table
    acpi::MADT* madt = (acpi::MADT*)acpi::findTable("APIC");


    // Do two passes. First pass is counting, second pass is setting
    multi::nCpus = 0;

    acpi::MADTHeader* curr = madt->contents;
    while ((uint64_t)curr < ((uint64_t)madt)+madt->header.Length) {
        switch (curr->type) {
            case acpi::ProcessorLocalApic : {
                // check validity
                ProcessorLocalApic* pla = (ProcessorLocalApic*)curr;
                if (pla->flags & 0b11)
                    multi::nCpus += 1;
                break;
            }
            case acpi::LocalApicAddressOverride : {
                panic("Local APIC address overriden");
                break;
            }
            case acpi::ProcessorLocalx2Apic : {
                // check validity
                ProcessorLocalx2Apic* pla = (ProcessorLocalx2Apic*)curr;
                if (pla->flags & 0b11)
                    multi::nCpus += 1;
                break;
            }
            default : {
                break;
            }
        }
        *((uint64_t*)&curr) += curr->length;
    }

    multi::cpus = (multi::CPU*)kmalloc(sizeof(multi::CPU)*multi::nCpus);

    uint64_t currCpu = 0;
    curr = madt->contents;
    while (currCpu < multi::nCpus) {
        switch (curr->type) {
            case acpi::ProcessorLocalApic : {
                ProcessorLocalApic* pla = (ProcessorLocalApic*)curr;
                multi::cpus[currCpu].acpiId = pla->acpiProcessorId;
                multi::cpus[currCpu].apicId = pla->apicId;
                currCpu++;
                break;
            }
            case acpi::ProcessorLocalx2Apic : {
                ProcessorLocalx2Apic* pla = (ProcessorLocalx2Apic*)curr;
                multi::cpus[currCpu].acpiId = pla->acpiProcessorId;
                multi::cpus[currCpu].apicId = pla->apicId;
                currCpu++;
                break;
            }
            default : {
                break;
            }
        }
        *((uint64_t*)&curr) += curr->length;
    }

    // mark the lapic space as reserved + map it
    // 0x3F0 from https://wiki.osdev.org/APIC#Local_APIC_registers
    memory::kernelIdentityMap4KiBBlock(LAPIC_BASE, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_WRITE_THROUGH);
    memory::markBlockAsUsed(LAPIC_BASE, 4*KiB);
    // init the lapic variable
    // c++ is a bit of a mess, so i need to hackily memcpy it into the right place
    // LapicRegisters localLapic = LapicRegisters(localApicPhysAddr);
    // memcpy(&lapic, &localLapic, sizeof(LapicRegisters));

    registerInterruptHandler(0xff, spurious, false);
    registerInterruptHandler(48, timer, false);

    // set the siv
    *lapic::spuriousInterruptVector = 0x1ff;
    // enable all interrupts
    *lapic::taskPriority = 0;

    // now we try and figure out the frequency of the APIC
    // get crystal + bus frequency respectively
    unsigned int crystal, bus, eax, unused;
    __cpuid(0x15, eax, unused, crystal, unused);
    __cpuid(0x16, eax, unused, bus, unused);
    
    // QEMU doesn't give me the real thing so i guess we won't
    // TODO: fix this before we start on real hardware

    *lapic::timerDivideConfiguration = 0x3;
    lapic::lvt::timer->clear();
    // *lapic::timerInitialCount = 0xffffff;

    // we need to actually measure the APIC timer freq using the PIT
    // initialise the PIT + set it up for hardware oneshot
    // some stuff about channel 2 gate status
    uint8_t gs = io::inb(0x61);
    // set bit 1 low, 0 high
    // im not sure why we need to set bit 1 low, i think it disables the speaker
    io::outb(0x61, (gs & (~0b10)) | 0b1);
    // channel 2, hw one shot
    io::outb(0x43, 0b10110010);
    // we want, say, 100Hz, so
    uint16_t div = 1193180/100;
    // write lo
    io::outb(0x42, div&0xff);
    // short delay, garbage read
    io::inb(0x40);
    // write high
    io::outb(0x42, (div>>8)&0xff);
    // reset the APIC timer
    *lapic::timerInitialCount = -1;
    // reset the timer (start counting)
    // gate low
    gs = io::inb(0x61);
    io::outb(0x61, gs & (~0b1));
    // gate high
    io::outb(0x61, gs | 0b1);
    // enable the apic counter
    *(uint32_t*)lapic::lvt::timer |= 0x00020000;
    // now wait until the PIT reaches 0
    while ((io::inb(0x61) & 0x20) == 0) {}
    // get the current counter
    uint32_t currCount = *lapic::timerCurrentCount;
    // the ticks elapsed in 10ms
    ticksPerSecond = 100*(((uint32_t)-1) - currCount);
    // we did use 0x3 divisor but since we keep that we can ignore it
    // enable the timer again
    lapic::lvt::timer->vectorNumber = 48;
    *(uint32_t*)lapic::lvt::timer |= 0x00020000;

    setApicTimerHz(4);

    while(1) {};
}

void setApicTimerHz(uint64_t hz) {
    // ticksPerSecond/hz
    *lapic::timerInitialCount = ticksPerSecond/hz;
}

}