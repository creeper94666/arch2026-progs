#include "fixed.h"

uint32_t op1(uint32_t a, uint32_t b, int *n)
{
    int passes = 0;
    while (b) {
        uint32_t carry = (a & b) << 1;
        a ^= b;
        b = carry;
        ++passes;
    }
    *n = passes;
    return a;
}

uint32_t op2(uint32_t a, uint32_t b)
{
    return a - b;
}

uint32_t op3(uint32_t a, uint32_t b)
{
    return (a & b) + ((a ^ b) >> 1);
}

int32_t q_mul(int32_t a, int32_t b)
{
    uint32_t ua = (uint32_t) a, ub = (uint32_t) b;
    uint32_t lo = 0, hi = 0, xlo = ua, xhi = 0, y = ub;
    while (y) {
        if (y & 1u) {
            uint32_t next = lo + xlo;
            hi += xhi + (next < lo);
            lo = next;
        }
        xhi = (xhi << 1) | (xlo >> 31);
        xlo <<= 1;
        y >>= 1;
    }

    /* Convert the raw unsigned product to the signed product modulo 2^64. */
    if (a < 0)
        hi -= ub;
    if (b < 0)
        hi -= ua;

    uint32_t rounded = lo + (UINT32_C(1) << (Q - 1));
    hi += rounded < lo;
    /* Retained bits 20..51 do not depend on arithmetic-shift sign fill. */
    uint32_t result = (rounded >> Q) | (hi << (32 - Q));
    /* Portable two's-complement bit mapping: each signed addition fits. */
    return (int32_t) (result & 0x7fffffffu) + ((result >> 31) ? INT32_MIN : 0);
}
