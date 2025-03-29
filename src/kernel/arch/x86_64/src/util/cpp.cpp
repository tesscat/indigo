// some stuff c++ wants us to have

#include "util/util.hpp"
extern "C" void __cxa_pure_virtual() {
    panic("__cxa_pure_virtual called");
}
// have to dummy it else someone complains
extern "C" int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) {
    UNUSED(func);
    UNUSED(arg);
    UNUSED(dso_handle);
    return 0;
}
