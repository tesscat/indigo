#include "libstd/string.hpp"
#include "libmem/string.hpp"
#include "memory/heap.hpp"
#include <libmem/mem.hpp>
#include <io/iostream.hpp>

bool String::operator==(String& other) {
    for (uint64_t i = 0; i < len && i < other.len; i++) {
        if (other[i] != base[i]) return false;
    }
    return true;
}

char String::operator[](int n) {
    return base[n];
}

String::String(const char* a) {
    len = libmem::strlen(a);
    if (len != 0) {
        base = (char*)kmalloc(len+1);
        memcpy(base, a, len);
        base[len] = '\0';
    }
}

String::String() {
    len = 0;
    base = nullptr;
}