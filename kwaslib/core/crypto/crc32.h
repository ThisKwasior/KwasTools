#pragma once

#include <stdint.h>

#include "crc_utils.h"

/*
    https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
    https://wiki.osdev.org/CRC32
    https://crccalc.com
*/

/*
    CRC32 version specific defines
*/

/* Known as CRC-32, ISO-HDLC, ADCCP, V-42, XZ and PKZIP */
#define CRC32_POLY          ((uint32_t)0x04C11DB7)
#define CRC32_POLY_INV      ((uint32_t)0xEDB88320)
#define CRC32_INIT          ((uint32_t)0xFFFFFFFF)
#define CRC32_XOROUT        ((uint32_t)0xFFFFFFFF)
#define CRC32_REFIN         CRC_REFLECTION_TRUE
#define CRC32_REFOUT        CRC_REFLECTION_TRUE

/* Known as CRC-32C, ISCSI, BASE91-C, CASTAGNOLI, INTERLAKEN */
#define CRC32_C_POLY        ((uint32_t)0x1EDC6F41)
#define CRC32_C_POLY_INV    ((uint32_t)0x82F63B78)
#define CRC32_C_INIT        ((uint32_t)0xFFFFFFFF)
#define CRC32_C_XOROUT      ((uint32_t)0xFFFFFFFF)
#define CRC32_C_REFIN       CRC_REFLECTION_TRUE
#define CRC32_C_REFOUT      CRC_REFLECTION_TRUE

/*
    Implementation
*/

/*
    Fits-all function to calculate any CRC32 with any
    inverted polynomial, init, xorout values and reflection params.
*/
uint32_t crc32_calc_hash_bit_by_bit(const uint8_t* data, const uint64_t size,
                                    const uint32_t poly,
                                    const uint32_t init, const uint32_t xorout,
                                    const uint8_t refin, const uint8_t refout);

/*
    Calculates the value in the CRC32 lookup table for a specified byte.
*/
static inline uint32_t crc32_calc_table_entry(const uint32_t poly, const uint8_t index, const uint8_t refin)
{
    uint32_t crc = index;

    if(refin == CRC_REFLECTION_TRUE) crc = crc_reflect_u8(crc);
    
    crc <<= 24;
    
    for(uint8_t i = 0; i < 8; ++i)
    {
        const uint32_t test = crc & 0x80000000;
        crc <<= 1;
        if(test) crc ^= poly;
    }
    
    if(refin == CRC_REFLECTION_TRUE) crc = crc_reflect_u32(crc);
    
    return crc;
}

/*
    Quick way for calculating CRC-32
*/
static inline uint32_t crc32_encode(const uint8_t* data, const uint64_t size)
{
    return crc32_calc_hash_bit_by_bit(data, size,
                                      CRC32_POLY, CRC32_INIT, CRC32_XOROUT,
                                      CRC32_REFIN, CRC32_REFOUT);
}

/*
    Quick way for calculating CRC-32C
*/
static inline uint32_t crc32_encode_crc32c(const uint8_t* data, const uint64_t size)
{
    return crc32_calc_hash_bit_by_bit(data, size,
                                      CRC32_C_POLY, CRC32_C_INIT, CRC32_C_XOROUT,
                                      CRC32_C_REFIN, CRC32_C_REFOUT);
}