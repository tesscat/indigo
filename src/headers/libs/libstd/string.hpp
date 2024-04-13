#ifndef LIBS_LIBSTD_STRING_HPP
#define LIBS_LIBSTD_STRING_HPP

#include <stdint.h>

namespace io {class sstream;}

class String {
    char* base;
public:
    uint64_t len = 0;
    String();
    String(const char*);
    bool operator==(String& other);
    char operator[](int n);
};

#endif // !LIBS_LIBSTD_STRING_HPP
