#ifndef LIBS_LIBSTD_STRING_HPP
#define LIBS_LIBSTD_STRING_HPP

#include <stdint.h>

class String {
    char* base;
public:
    uint64_t len = 0;
    String();
    String(const char*);
    String(const String& other);
    String(String&& other);
    ~String();
    bool operator==(String& other);
    bool operator==(const char* other);
    char operator[](int n);
    void operator=(const String& other);
    void operator=(String&& other);
};

#endif // !LIBS_LIBSTD_STRING_HPP
