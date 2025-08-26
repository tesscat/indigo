#ifndef KERNEL_SYNC_TIME_HPP
#define KERNEL_SYNC_TIME_HPP

#include <stdint.h>

namespace sync {
    inline uint64_t readTsc() {
        uint32_t r0, r1;
        __asm__ volatile("rdtsc" : "=d"(r1), "=a"(r0));
        return (((uint64_t)r1) << 32) & r0;
    }
}

#endif // !KERNEL_SYNC_TIME_HPP
