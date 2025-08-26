#ifndef KERNEL_APIC_PIC_HPP
#define KERNEL_APIC_PIC_HPP

// https://wiki.osdev.org/PIC

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#include "io/io.hpp"
namespace apic::pic {
    inline void disablePic() {
        // remask all the interrupts anyway to fully ensure they don't mess us
        // up if they erroneously fire start PIC init sequence
        io::outb(PIC1_DATA, ICW1_INIT | ICW1_ICW4);
        io::outb(PIC2_DATA, ICW1_INIT | ICW1_ICW4);
        // set offsets
        // we have exceptions from 0->31, so we use 32->39 and 40->48
        // TODO: can we map them on top of each other? should we?
        io::outb(PIC1_DATA, 32);
        io::outb(PIC2_DATA, 40);
        // mask all the interrupts
        io::outb(PIC1_DATA, 0xff);
        io::outb(PIC2_DATA, 0xff);
    }
}

#endif // !KERNEL_APIC_PIC_HPP
