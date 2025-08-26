#ifndef KERNEL_APIC_GDT_HPP
#define KERNEL_APIC_GDT_HPP

#include <stdint.h>

namespace apic {
    // Size + offset are ignored on x86_64
    struct GdtDescriptor {
        uint16_t size   : 16;
        uint16_t offset : 16;
        uint8_t offset2 : 8;
        // // should leave as 1 to stop CPU trying to write this
        // uint8_t accessed : 1 = 1;
        // // readable for code segs, writable for data segs
        // uint8_t readOrWrite : 1;
        // // for data segs kinda irrelevant,
        // // for code segs allows code in this segment to be executed from an
        // equal or lower privelege level
        // // or else it'd just be the DPL ring
        // // the priv level does stay the same despite the jump
        // uint8_t directionOrConforming : 1;
        // // data or code seg?
        // uint8_t executable : 1;
        // // is this a task state seg?
        // uint8_t isNotTSS : 1 = 0;
        // uint8_t dpl : 2;
        // // is this actually a valid seg?
        // uint8_t present : 1;
        uint8_t accessByte : 8;
        uint8_t size2      : 4;
        // uint8_t reserved : 1;
        // // only 1 if this is a code seg and 64-bit code
        // // if set, DB should be unset
        // uint8_t isLongModeCode : 1;
        // // is this 16 or 32 bits? (we don't really care for data segs)
        // uint8_t sizeFlag : 1;
        // // irr for long mode
        // uint8_t granularity : 1;
        uint8_t flags   : 4;
        uint8_t offset3 : 8;
        inline void clear() {
            *(uint64_t*)(this) = 0;
        }
    } __attribute__((packed));
    struct GdtPointer {
        uint16_t size   : 16;
        uint64_t offset : 64;
    } __attribute__((packed));

    struct SegmentSelector {
        uint8_t requestedPrivLevel : 2;
        uint8_t useLDT             : 1 = 0;
        uint16_t indexHigh         : 13;
        inline void setIndex(uint16_t index) {
            indexHigh = index >> 3;
        }
    } __attribute__((packed));

    void initGdt();
    void loadGdt();

}

#endif // !KERNEL_APIC_GDT_HPP
