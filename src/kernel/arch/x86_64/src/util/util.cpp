#include <util/util.hpp>

namespace util {

// [[noreturn]] void _panic(const char *file, const char *line, const char *func) noexcept {
//     // logs::info << "\nPANIC! at " << file << ":" << line << " (`" << func << "`): " << msg << '\n';
//     loop_forever;
// }
[[noreturn]] void _panic(const char* msg, const char *file, const char *line, const char *func) noexcept {
    logs::info << "\nPANIC! at " << file << ":" << line << " (`" << func << "`): " << msg << '\n';
    loop_forever;
}
}
