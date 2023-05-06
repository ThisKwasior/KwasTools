#pragma once

/*
    Set of functions for reading LE and BE types.
    
    CryTools: Will replace "common.h/.c" and "io_common.h/.c"
*/

#include <stdint.h>

#define BIT(x) (1<<(x))

/*
    Check endianness
*/
uint8_t tr_is_BE();

/*
    Changes endianness of the number
*/
uint16_t tr_swap_endian_16(const uint16_t n);
uint32_t tr_swap_endian_32(const uint32_t n);
uint64_t tr_swap_endian_64(const uint64_t n);

/*
    Readers for any endianness
*/
uint8_t tr_read_u8(const uint8_t* data);
void tr_read_array(const uint8_t* data, const uint64_t len, uint8_t* buffer);
uint8_t* tr_read_array_alloc(const uint8_t* data, const uint64_t len);

/*
    Readers for host endianness.
    Doesn't check for endianness, just reads data.
*/
uint16_t tr_read_u16(const uint8_t* data);
uint32_t tr_read_u32(const uint8_t* data);
uint64_t tr_read_u64(const uint8_t* data);
float tr_read_f32(const uint8_t* data);
double tr_read_f64(const uint8_t* data);

/*
    Readers for little endian
*/
uint16_t tr_read_u16le(const uint8_t* data);
uint32_t tr_read_u32le(const uint8_t* data);
uint64_t tr_read_u64le(const uint8_t* data);
float tr_read_f32le(const uint8_t* data);
double tr_read_f64le(const uint8_t* data);

/*
    Readers for big endian
*/
uint16_t tr_read_u16be(const uint8_t* data);
uint32_t tr_read_u32be(const uint8_t* data);
uint64_t tr_read_u64be(const uint8_t* data);
float tr_read_f32be(const uint8_t* data);
double tr_read_f64be(const uint8_t* data);