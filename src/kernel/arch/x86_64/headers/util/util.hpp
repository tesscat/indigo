#ifndef KERNEL_UTIL_UTIL_HPP
#define KERNEL_UTIL_UTIL_HPP

#include <stdint.h>

namespace util {

#define MiB (1024*1024)
#define KiB (1024)

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

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
}

#endif // !KERNEL_UTIL_UTIL_HPP
