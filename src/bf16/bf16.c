#include "bf16.h"

uint16_t bf16_from_f32(uint32_t bits)
{
    if (((bits >> 23) & 255u) != 255u)
        bits += 0x7fffu + ((bits >> 16) & 1u);
    return (uint16_t) (bits >> 16);
}

static uint32_t multiply8(uint32_t a, uint32_t b)
{
    uint32_t product = 0;
    do {
        if (b & 1u)
            product += a;
        a <<= 1;
        b >>= 1;
    } while (b);
    return product;
}

uint16_t bf16_mul(uint16_t a, uint16_t b)
{
    uint32_t sign = (a ^ b) & 0x8000u;
    uint32_t ma = a & 127u, mb = b & 127u;
    int32_t ea = (a >> 7) & 255u, eb = (b >> 7) & 255u;

    /* The contract examines a first: an infinite a even swallows a NaN b. */
    if (ea == 255) {
        if (ma)
            return a;
        if ((b & 0x7fffu) == 0)
            return 0x7fc0;
        return (uint16_t) (sign | 0x7f80u);
    }
    if (eb == 255) {
        if (mb)
            return b;
        if ((a & 0x7fffu) == 0)
            return 0x7fc0;
        return (uint16_t) (sign | 0x7f80u);
    }
    if ((a & 0x7fffu) == 0 || (b & 0x7fffu) == 0)
        return (uint16_t) sign;

    if (ea == 0) {
        ea = 1;
        do {
            ma <<= 1;
            --ea;
        } while (ma < 128u);
    } else
        ma |= 128u;
    if (eb == 0) {
        eb = 1;
        do {
            mb <<= 1;
            --eb;
        } while (mb < 128u);
    } else
        mb |= 128u;

    uint32_t product = multiply8(ma, mb);
    int32_t exponent = ea + eb - 127;
    if (product >= 32768u) {
        product >>= 1;
        ++exponent;
    }
    uint32_t fraction = (product >> 7) & 127u;
    if (exponent >= 255)
        return (uint16_t) (sign | 0x7f80u);
    if (exponent <= 0) {
        if (exponent < -6)
            return (uint16_t) sign;
        /* Course rule: remove the hidden bit BEFORE the underflow shift. */
        return (uint16_t) (sign | (fraction >> (1 - exponent)));
    }
    return (uint16_t) (sign | ((uint32_t) exponent << 7) | fraction);
}

uint16_t bf16_sqrt(uint16_t a)
{
    uint32_t exponent = (a >> 7) & 255u, fraction = a & 127u;
    if (exponent == 255u) {
        if (fraction)
            return a;
        return (a & 0x8000u) ? 0x7fc0 : a;
    }
    if ((a & 0x7fffu) == 0)
        return 0;
    if (a & 0x8000u)
        return 0x7fc0;
    if (exponent == 0)
        return 0;

    uint32_t mantissa = fraction | 128u;
    if ((exponent & 1u) == 0)
        mantissa <<= 1;

    /* floor(r*r/128)<=m iff r*r<=128*m+127. The +127 preserves the
     * specified truncating search, rather than an IEEE-rounded sqrt. */
    uint32_t stream = ((mantissa << 7) | 127u) << 16;
    uint32_t root = 0, remainder = 0;
    for (unsigned i = 0; i < 8; ++i) {
        remainder = (remainder << 2) | (stream >> 30);
        stream <<= 2;
        root <<= 1;
        uint32_t trial = (root << 1) | 1u;
        if (remainder >= trial) {
            remainder -= trial;
            ++root;
        }
    }
    /* Raw exponent parity avoids signed shifts of negative exponents. */
    return (uint16_t) ((((exponent + 127u) >> 1) << 7) | (root & 127u));
}
