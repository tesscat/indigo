#ifndef KERNEL_MULTI_CPU_HPP
#define KERNEL_MULTI_CPU_HPP

#include <stdint.h>

namespace multi {

struct CPU {
    uint32_t apicId;
    uint32_t acpiId;
};

extern uint64_t nCpus;
extern CPU* cpus;

inline uint16_t getCpuIdx() {
    uint16_t ret = 0;
    __asm__ volatile ("movw %%fs, %0" : "=rim"(ret));
    return ret ;
}

}

#endif // !KERNEL_MULTI_CPU_HPP
