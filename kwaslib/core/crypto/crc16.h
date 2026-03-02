#pragma once

#include <stdint.h>

#include "crc_utils.h"

/*
    https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
    https://wiki.osdev.org/CRC32
    https://crccalc.com
    https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.crc-16-umts
*/

/*
    CRC16 version specific defines
*/

/* Known as UMTS, BUYPASS, VERIFONE. Used for RS232/RS485 communication. */
#define CRC16_UMTS_POLY           ((uint16_t)0x8005)
#define CRC16_UMTS_POLY_INV       ((uint16_t)0xA001)
#define CRC16_UMTS_INIT           ((uint16_t)0x0000)
#define CRC16_UMTS_XOROUT         ((uint16_t)0x0000)
#define CRC16_UMTS_REFIN          CRC_REFLECTION_FALSE
#define CRC16_UMTS_REFOUT         CRC_REFLECTION_FALSE

/*
    Implementation
*/

/*
    Fits-all function to calculate any CRC16 with any
    inverted polynomial, init, xorout values and reflection params.
*/
uint16_t crc16_calc_hash_bit_by_bit(const uint8_t* data, const uint64_t size,
                                    const uint16_t poly,
                                    const uint16_t init, const uint16_t xorout,
                                    const uint8_t refin, const uint8_t refout);

/*
    Quick way for calculating CRC-16-UMTS
*/
static inline uint16_t crc16_encode_umts(const uint8_t* data, const uint64_t size)
{
    return crc16_calc_hash_bit_by_bit(data, size,
                                      CRC16_UMTS_POLY,
                                      CRC16_UMTS_INIT, CRC16_UMTS_XOROUT,
                                      CRC16_UMTS_REFIN, CRC16_UMTS_REFOUT);
}