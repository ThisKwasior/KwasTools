#pragma once

#include <stdint.h>

/*
    Check endianness
*/
uint8_t ed_is_BE();

/*
    Changes endianness of the number
*/
uint16_t ed_swap_endian_16(const uint16_t n);
uint32_t ed_swap_endian_32(const uint32_t n);
uint64_t ed_swap_endian_64(const uint64_t n);