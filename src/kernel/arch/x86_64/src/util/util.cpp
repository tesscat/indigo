#include "logs/logs.hpp"
#include <util/util.hpp>

namespace util {

[[noreturn]] void _panic(const char *msg, const char *file, const char *line, const char *func) noexcept {
    logs::info << "\nPANIC! at " << file << ":" << line << " (`" << func << "`): " << msg;
    loop_forever;
}

}
