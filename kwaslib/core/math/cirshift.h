#pragma once

#include <stdint.h>

/*
    https://en.wikipedia.org/wiki/Circular_shift
    Also known as bitwise rotation (rotl/rotr)
*/

/*
    Circular shift implementations by John Regehr and Peter Cordes.
    
    I removed the need for limits.h - get a real computer with CHAR_BIT==8
*/
static inline uint32_t cirshift_l32(uint32_t value, uint32_t count)
{
    const uint32_t mask = 8 * sizeof(value) - 1;
    count &= mask;
    return (value << count) | (value >> (-count & mask));
}

static inline uint32_t cirshift_r32(uint32_t value, uint32_t count)
{
    const uint32_t mask = 8 * sizeof(value) - 1;
    count &= mask;
    return (value >> count) | (value << (-count & mask));
}

static inline uint64_t cirshift_l64(uint64_t value, uint64_t count)
{
    const uint64_t mask = 8 * sizeof(value) - 1;
    count &= mask;
    return (value << count) | (value >> (-count & mask));
}

static inline uint64_t cirshift_r64(uint64_t value, uint64_t count)
{
    const uint64_t mask = 8 * sizeof(value) - 1;
    count &= mask;
    return (value >> count) | (value << (-count & mask));
}
