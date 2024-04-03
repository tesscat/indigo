#include "graphics/psf.hpp"
#include <lai/host.h>
#include <util/util.hpp>

// avoid c++ name manglage
extern "C" {
void laihost_log(int _level, const char* msg) {
    graphics::psf::print("LAI: ");
    graphics::psf::print(msg);
}

void laihost_panic(const char* msg) {
    graphics::psf::print("PANIC (LAI): ");
    graphics::psf::print(msg);
    loop_forever;
}
}
