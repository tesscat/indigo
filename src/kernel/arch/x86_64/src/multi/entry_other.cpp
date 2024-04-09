#include "graphics/psf.hpp"
#include "util/util.hpp"
#include <multi/start_others.hpp>

extern "C" void entry_other() {
    // properly paged not proper GDT'd loaded
    graphics::psf::print("hello from other\n");
    // tell BSP we are living
    multi::startupLock.release();
    loop_forever;
}
