#pragma once

/*
    https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
    https://wiki.osdev.org/CRC32
    https://crccalc.com
*/

#include <stdint.h>

/*
    Common defines
*/
#define CRC_REFLECTION_TRUE     (uint8_t)(1)
#define CRC_REFLECTION_FALSE    (uint8_t)(0)

/*
    Implementation
*/

/*
    Just inverts the byte's bits front to back.
*/
static inline uint8_t crc_reflect_u8(const uint8_t u8)
{
    uint8_t result = 0;
    
    for(uint8_t i = 0; i != 8; ++i)
    {
        uint8_t bit = (u8>>i)&1;
        result |= (bit<<(7-i));
    }

    return result;
}

static inline uint16_t crc_reflect_u16(const uint16_t u16)
{
    uint16_t result = 0;
    
    for(uint8_t i = 0; i != 16; ++i)
    {
        uint16_t bit = (u16>>i)&1;
        result |= (bit<<(15-i));
    }

    return result;
}

static inline uint32_t crc_reflect_u32(const uint32_t u32)
{
    uint32_t result = 0;
    
    for(uint8_t i = 0; i != 32; ++i)
    {
        uint32_t bit = (u32>>i)&1;
        result |= (bit<<(31-i));
    }
    
    return result;
}