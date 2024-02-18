#include "loader/kernel_args.hpp"

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;

extern char _tramp_start;
extern char _tramp_end;

extern "C" void _start(KernelArgs* args) {
    kargs = args;
    volatile uint64_t __attribute((__used__)) ts = (uint64_t) &_tramp_start;
    uint64_t te = (uint64_t) _tramp_end;
    // set up a nice pagemap
    // identity map wherever we are 
    loop_forever;
}
