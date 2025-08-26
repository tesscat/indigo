#ifndef KERNEL_SYNC_SPINLOCK_HPP
#define KERNEL_SYNC_SPINLOCK_HPP

#include <stdint.h>

namespace sync {

    struct Spinlock;
    extern "C" void __lockSpinlock(Spinlock* lock);
    extern "C" void __releaseSpinlock(Spinlock* lock);
    extern "C" void __awaitSpinlockRelease(Spinlock* lock);

    struct Spinlock {
        volatile uint32_t _ = 0;
        inline void init() {
            _ = 0;
        }
        inline void lock() {
            __lockSpinlock(this);
        }
        inline void release() {
            __releaseSpinlock(this);
        }
        // Waits for the lock to be released
        inline void await() {
            __awaitSpinlockRelease(this);
        }
    } __attribute__((packed));

}

#endif // !KERNEL_SYNC_SPINLOCK_HPP
