#ifndef KERNEL_IO_OSTREAM_HPP
#define KERNEL_IO_OSTREAM_HPP

#include "libstd/string.hpp"
namespace io {
class sstream {
public:
    virtual void write(String& text) =0;
    virtual void write(const char* text) =0;
    virtual void write(const char c) =0;
    virtual ~sstream() =default;
};

class cout_t : public sstream {
public:
    virtual void write(String& text) override;
    virtual void write(const char* text) override;
    virtual void write(const char c) override;
    virtual ~cout_t() override =default;
};

extern class cout_t cout;

inline sstream& operator<<(sstream& stream, const char* text) {stream.write(text); return stream;}
inline sstream& operator<<(sstream& stream, String& text) {stream.write(text); return stream;}
inline sstream& operator<<(sstream& stream, const char c) {stream.write(c); return stream;}
sstream& operator<<(sstream& stream, int64_t v);
sstream& operator<<(sstream& stream, uint64_t v);
inline sstream& operator<<(sstream& stream, int v) {return stream<<(int64_t)v;}
sstream& operator<<(sstream& stream, void* v);
}

#endif // !KERNEL_IO_OSTREAM_HPP
