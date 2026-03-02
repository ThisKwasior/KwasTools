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
    CRC8 version specific defines
*/

/*
    Implementation
*/

/*
    Fits-all function to calculate any CRC16 with any
    inverted polynomial, init, xorout values and reflection params.
*/
uint8_t crc8_calc_hash_bit_by_bit(const uint8_t* data, const uint64_t size,
                                  const uint8_t poly,
                                  const uint8_t init, const uint8_t xorout,
                                  const uint8_t refin, const uint8_t refout);