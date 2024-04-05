#include <util/util.hpp>

namespace util {

[[noreturn]] void _panic(const char *msg, const char *file, const char *line, const char *func) noexcept {
    graphics::psf::print("\nPANIC! at ");
    graphics::psf::print(file);
    graphics::psf::print(":");
    graphics::psf::print(line);
    graphics::psf::print(" (`");
    graphics::psf::print(func);
    graphics::psf::print("`): ");
    graphics::psf::print(msg);
    loop_forever;
}

}
