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

bool String::operator==(const char* other) {
    for (uint64_t i = 0; i < len && other[i]; i++) {
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

String::~String() {
    if (base) kfree(base);
}

// Copy ctor
String::String(const String& other) {
    base = (char*)kmalloc(other.len + 1);
    memcpy(base, other.base, other.len);
    base[other.len] = '\0';
    len = other.len;
}
// Move ctor
String::String(String&& other) {
    base = other.base;
    other.base = nullptr;
    len = other.len;
    other.len = 0;
}

// copy
void String::operator=(const String& other) {
    base = (char*)kmalloc(other.len + 1);
    memcpy(base, other.base, other.len);
    base[other.len] = '\0';
    len = other.len;
}
// move
void String::operator=(String&& other) {
    base = other.base;
    other.base = nullptr;
    len = other.len;
    other.len = 0;
}

String::String() {
    len = 0;
    base = nullptr;
}
