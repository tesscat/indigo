#include <stdint.h>

int itoa(int64_t value, char* sp, int radix) {
    char tmp[22]; // be careful with the length of the buffer
    char* tp = tmp;
    int64_t i;
    uint64_t v;

    int sign = (radix == 10 && value < 0);
    if (sign) v = -value;
    else v = (unsigned)value;

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix;
        if (i < 10) *tp++ = i + '0';
        else *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp) *sp++ = *--tp;

    return len;
}
