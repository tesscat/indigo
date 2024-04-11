#ifndef LIBS_LIBSTD_STRING_HPP
#define LIBS_LIBSTD_STRING_HPP

#include <stdint.h>

class String {
    char* base;
public:
    uint64_t len;
    String() = delete;
    String(const char*);
    bool operator==(String& other);
    char operator[](int n);
};

#endif // !LIBS_LIBSTD_STRING_HPP
