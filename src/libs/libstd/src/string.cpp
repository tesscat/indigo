#include "libstd/string.hpp"
#include "libmem/string.hpp"
#include "memory/heap.hpp"
#include <libmem/mem.hpp>

bool String::operator==(String& other) {
    for (int i = 0; i < len && i < other.len; i++) {
        if (other[i] != base[i]) return false;
    }
    return true;
}

char String::operator[](int n) {
    return base[n];
}

String::String(const char* a) {
    len = libmem::strlen(a);
    base = (char*)kmalloc(len);
    memcpy(base, a, len);
}
