#ifndef KERNEL_INTERFACES_RW_HPP
#define KERNEL_INTERFACES_RW_HPP

#include <stdint.h>

namespace interfaces {
    class Reader {
    public:
        virtual ~Reader() {};
        virtual uint64_t size() = 0;
        // TODO: should i return a ReadWrapper instead or smth? so the reader
        // can manage frees instead?
        //
        // for the time being you need to free() this
        virtual uint8_t* read(uint64_t offs, uint64_t len) = 0;
        // TODO: other variants of read
    };
    class Writer {
    public:
        virtual ~Writer() {};
        // TODO: this class
    };
}
#endif // !KERNEL_INTERFACES_RW_HPP
