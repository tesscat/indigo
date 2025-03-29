#include "io/iostream.hpp"
#include "graphics/psf.hpp"
#include <libstd/itoa.hpp>

namespace io {

void cout_t::write(String& text) {
    for (uint64_t i = 0; i < text.len; i++) {
        graphics::psf::putchar(text[i]);
    }
};
void cout_t::write(const char* text) {
    graphics::psf::print(text);
}
void cout_t::write(const char c) {
    graphics::psf::putchar(c);
}

sstream& operator<<(sstream& stream, int64_t v) {
    char sp[25];
    int l = itoa(v, sp);
    sp[l] = '\0';
    stream.write(sp);
    return stream;
}
sstream& operator<<(sstream& stream, uint64_t v) {
    char sp[25];
    int l = itoa(v, sp);
    sp[l] = '\0';
    stream.write(sp);
    return stream;
}

sstream& operator<<(sstream& stream, void* v) {
    char sp[25];
    int l = itoa((uint64_t)v, sp, 16);
    sp[l] = '\0';
    stream.write("0x");
    stream.write(sp);
    return stream;
}

class cout_t cout;

}
