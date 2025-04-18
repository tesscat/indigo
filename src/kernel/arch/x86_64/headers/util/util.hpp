#ifndef KERNEL_UTIL_UTIL_HPP
#define KERNEL_UTIL_UTIL_HPP

#include <stdint.h>
#include <logs/logs.hpp>

namespace util {

#define MiB (1024*1024)
#define KiB (1024)

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}
#define UNUSED(x) ((void)(x))

#define __COMPTIME_STRINGIFY(X) __COMPTIME_STRINGIFY2(X)
#define __COMPTIME_STRINGIFY2(X) #X
// #define panic(msg) {logs::info << "PANIC! at" << __FILE__ << ":" << __COMPTIME_STRINGIFY(__LINE__) << " (`" << __PRETTY_FUNCTION__ << "`): " << msg << "\n"; \
    // util::_panic(__FILE__, __COMPTIME_STRINGIFY(__LINE__), __PRETTY_FUNCTION__);};
#define panic(msg) util::_panic(msg, __FILE__, __COMPTIME_STRINGIFY(__LINE__), __PRETTY_FUNCTION__);

#define unimplemented() panic("UNIMPLEMENTED")

// [[noreturn]] void _panic(const char* file, const char* line, const char* func) noexcept;
[[noreturn]] void _panic(const char* msg, const char* file, const char* line, const char* func) noexcept;

// returns ceil(a/b)
inline uint64_t ceilDiv(uint64_t a, uint64_t b) {
    return (a + b - 1)/b;
}

/**
 * @brief Rounds away-from-zero
 *
 * @param numToRound The number to round
 * @param multipleToRoundTo The multiple to round to
 * @return The rounded number
 */
inline uint64_t roundUp(uint64_t numToRound, uint64_t multipleToRoundTo) {
    if (multipleToRoundTo == 0) return numToRound;
    return ((numToRound + multipleToRoundTo - 1) / multipleToRoundTo) * multipleToRoundTo;
}
/**
 * @brief Rounds away-from-zero like `roundUp`, but is faster when multipleToRoundTo is a power of 2
 *
 * @param numToRound The number to round
 * @param multipleToRoundTo The multiple to round to
 * @return The rounded number
 */
inline uint64_t roundUpToPowerOfTwo(uint64_t numToRound, uint64_t multipleToRoundTo) {
    // check we have not been lied to
    if (!(multipleToRoundTo && ((multipleToRoundTo & (multipleToRoundTo - 1)) == 0)))
        return roundUp(numToRound, multipleToRoundTo);
    return (numToRound + multipleToRoundTo - 1) & (-multipleToRoundTo);
}

#define MSR_FSBASE 0xC0000100
inline void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi) {
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

inline void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi) {
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
template<typename A, typename B>
struct Pair {
    A first;
    B second;
};

template<typename A>
struct Optional {
    bool hasVal;
    union {uint8_t _; A val;} data;
    A get() {return data.val;}
    Optional() : hasVal{false} {}
    Optional(A val_) : hasVal{true}, data{val_} {}
};
}

#endif // !KERNEL_UTIL_UTIL_HPP
