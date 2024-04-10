#ifndef KERNEL_MULTI_CPU_HPP
#define KERNEL_MULTI_CPU_HPP

#include "util/util.hpp"
#include <stdint.h>

namespace multi {

struct CPU {
    uint32_t apicId;
    uint32_t acpiId;
};

extern uint64_t nCpus;
extern CPU* cpus;

inline uint64_t getCpuIdx() {
    uint32_t retlo, rethi = 0;
    util::cpuGetMSR(MSR_FSBASE, &retlo, &rethi);
    return (((uint64_t)rethi) << 32) | retlo;
}

}

#endif // !KERNEL_MULTI_CPU_HPP
