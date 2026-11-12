#include "gcd.h"

uint32_t gcd_u32(uint32_t a, uint32_t b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;

    unsigned common_twos = 0;
    while (((a | b) & 1u) == 0) {
        a >>= 1;
        b >>= 1;
        ++common_twos;
    }

    while ((a & 1u) == 0)
        a >>= 1;

    do {
        /* b is nonzero here; a stays odd throughout the loop. */
        while ((b & 1u) == 0)
            b >>= 1;
        if (a > b) {
            uint32_t temporary = a;
            a = b;
            b = temporary;
        }
        b -= a;
    } while (b != 0);

    return a << common_twos;
}
