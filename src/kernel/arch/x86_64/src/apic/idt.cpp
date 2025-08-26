#include <apic/idt.hpp>

namespace apic {
    __attribute__((aligned(0x10))) IdtGate idt[256];
    IdtDescriptor desc;

    void initIdt() {
        for (int i = 0; i < 256; i++) {
            idt[i].clear();
        }
        desc.size = sizeof(idt) - 1;
        // paging applies for IDT stuff
        desc.offset = (uint64_t)&idt[0];
        enableIdt();
    }
    void enableIdt() {
        __asm__ volatile("lidt %0" : : "m"(desc));
    }

    void registerCatchHandlerLogic(
        uint64_t idx,
        void* func,
        bool isTrap,
        uint8_t dpl
    ) {
        idt[idx].clear();
        idt[idx].setAddr((uint64_t)func);
        idt[idx].gateType = isTrap ? 0xF : 0xE;
        idt[idx].dpl      = dpl;
        idt[idx].present  = 1;
        idt[idx].segsel.setIndex(0x08); // kernel selector
        idt[idx].segsel.useLDT             = 0;
        idt[idx].segsel.requestedPrivLevel = 0;
    }

    void registerExceptionHandler(
        uint64_t idx,
        void (*func)(InterruptFrame*, uint64_t),
        bool isTrap,
        uint8_t dpl
    ) {
        registerCatchHandlerLogic(idx, (void*)func, isTrap, dpl);
    }

    void registerInterruptHandler(
        uint64_t idx,
        void (*func)(InterruptFrame*),
        bool isTrap,
        uint8_t dpl
    ) {
        registerCatchHandlerLogic(idx, (void*)func, isTrap, dpl);
    }

}
