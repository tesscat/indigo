#ifndef KERNEL_APIC_PIC_HPP
#define KERNEL_APIC_PIC_HPP

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#include "io/io.hpp"
namespace apic::pic {
inline void disablePic() {
    io::outb(PIC1_DATA, 0xff);
    io::outb(PIC2_DATA, 0xff);
}
}

#endif // !KERNEL_APIC_PIC_HPP
