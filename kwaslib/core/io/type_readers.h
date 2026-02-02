#pragma once

/*
    Set of functions for reading LE and BE types.
    
    CryTools: Will replace "common.h/.c" and "io_common.h/.c"
*/

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/cpu/endianness.h>

#define BIT(x) (1<<(x))

/*
    Readers for any endianness
*/
static inline const uint8_t tr_read_u8(const uint8_t* data)
{
    return data[0];
}

static inline void tr_read_array(const uint8_t* data, const uint64_t len, uint8_t* buffer)
{
    memcpy(buffer, data, len);
}

static inline uint8_t* tr_read_array_alloc(const uint8_t* data, const uint64_t len)
{
    uint8_t* arr = (uint8_t*)calloc(1, len);
    memcpy(arr, data, len);
    return arr;
}

/*
    Readers for host endianness.
    Doesn't check for endianness, just reads data.
*/
static inline const uint16_t tr_read_u16(const uint8_t* data)
{
    return (*(uint16_t*)&data[0]);
}

static inline const uint32_t tr_read_u32(const uint8_t* data)
{
    return (*(uint32_t*)&data[0]);
}

static inline const uint64_t tr_read_u64(const uint8_t* data)
{
    return (*(uint64_t*)&data[0]);
}

static inline const float tr_read_f32(const uint8_t* data)
{
    return (*(float*)&data[0]);
}

static inline const double tr_read_f64(const uint8_t* data)
{
    return (*(double*)&data[0]);
}

/*
    Readers for little endian
*/
static inline const uint16_t tr_read_u16le(const uint8_t* data)
{
    uint16_t n = tr_read_u16(data);
    if(ed_is_BE() == ED_ENDIAN_BIG) n = ed_swap_endian_16(n);
    return n;
}

static inline const uint32_t tr_read_u32le(const uint8_t* data)
{
    uint32_t n = tr_read_u32(data);
    if(ed_is_BE() == ED_ENDIAN_BIG) n = ed_swap_endian_32(n);
    return n;
}

static inline const uint64_t tr_read_u64le(const uint8_t* data)
{
    uint64_t n = tr_read_u64(data);
    if(ed_is_BE() == ED_ENDIAN_BIG) n = ed_swap_endian_64(n);
    return n;
}

static inline const float tr_read_f32le(const uint8_t* data)
{
    uint32_t n = tr_read_u32(data);
    if(ed_is_BE() == ED_ENDIAN_BIG) n = ed_swap_endian_32(n);
	const float x = *(float*)&n;
    return x;
}

static inline const double tr_read_f64le(const uint8_t* data)
{
    uint64_t n = tr_read_u64(data);
    if(ed_is_BE() == ED_ENDIAN_BIG) n = ed_swap_endian_64(n);
	const double x = *(double*)&n;
    return x;
}

/*
    Readers for big endian
*/
static inline const uint16_t tr_read_u16be(const uint8_t* data)
{
    uint16_t n = tr_read_u16(data);
    if(ed_is_BE() == ED_ENDIAN_LITTLE) n = ed_swap_endian_16(n);
    return n;
}

static inline const uint32_t tr_read_u32be(const uint8_t* data)
{
    uint32_t n = tr_read_u32(data);
    if(ed_is_BE() == ED_ENDIAN_LITTLE) n = ed_swap_endian_32(n);
    return n;
}

static inline const uint64_t tr_read_u64be(const uint8_t* data)
{
    uint64_t n = tr_read_u64(data);
    if(ed_is_BE() == ED_ENDIAN_LITTLE) n = ed_swap_endian_64(n);
    return n;
}

static inline const float tr_read_f32be(const uint8_t* data)
{
    uint32_t n = tr_read_u32(data);
    if(ed_is_BE() == ED_ENDIAN_LITTLE) n = ed_swap_endian_32(n);
	const float x = *(float*)&n;
    return x;
}

static inline const double tr_read_f64be(const uint8_t* data)
{
    uint64_t n = tr_read_u64(data);
    if(ed_is_BE() == ED_ENDIAN_LITTLE) n = ed_swap_endian_64(n);
	const double x = *(double*)&n;
    return x;
}