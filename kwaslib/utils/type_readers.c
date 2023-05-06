#include <string.h>
#include <stdlib.h>

#include "type_readers.h"

/*
    Check endianness
*/
uint8_t tr_is_BE()
{
    const uint16_t i = 0x0102;
    const uint8_t* ia = (const uint8_t*)&i;
    if(ia[0] == 0x01) return 1;
    return 0;
}

/*
    Changes endianness of the number
*/
uint16_t tr_swap_endian_16(const uint16_t n)
{
	return (n << 8) | (n >> 8);
}

uint32_t tr_swap_endian_32(const uint32_t n)
{
	return (n >> 24) | 
		   ((n & 0xFF0000) >> 8) |
		   ((n & 0xFF00) << 8) |
		   (n << 24);
}

uint64_t tr_swap_endian_64(const uint64_t n)
{
	return (n >> 56) | 
		   ((n & 0xFF000000000000) >> 40) |
		   ((n & 0xFF0000000000) >> 24) |
		   ((n & 0xFF00000000) >> 8) |
		   ((n & 0xFF00) << 40) |
		   ((n & 0xFF0000) << 24) |
		   ((n & 0xFF000000) << 8) |
		   (n << 56);
}

/*
    Readers for any endianness
*/
uint8_t tr_read_u8(const uint8_t* data)
{
    return data[0];
}

void tr_read_array(const uint8_t* data, const uint64_t len, uint8_t* buffer)
{
    memcpy(buffer, data, len);
}

uint8_t* tr_read_array_alloc(const uint8_t* data, const uint64_t len)
{
    uint8_t* arr = (uint8_t*)calloc(1, len);
    memcpy(arr, data, len);
    return arr;
}

/*
    Readers for host endianness.
    Doesn't check for endianness, just reads data.
*/
uint16_t tr_read_u16(const uint8_t* data)
{
    uint16_t n = *(uint16_t*)&data[0];
    return n;
}

uint32_t tr_read_u32(const uint8_t* data)
{
    uint32_t n = *(uint32_t*)&data[0];
    return n;
}

uint64_t tr_read_u64(const uint8_t* data)
{
    uint64_t n = *(uint64_t*)&data[0];
    return n;
}

float tr_read_f32(const uint8_t* data)
{
    float n = *(float*)&data[0];
    return n;
}

double tr_read_f64(const uint8_t* data)
{
    double n = *(double*)&data[0];
    return n;
}

/*
    Readers for little endian
*/
uint16_t tr_read_u16le(const uint8_t* data)
{
    uint16_t n = *(uint16_t*)&data[0];
    if(tr_is_BE() == 1) n = tr_swap_endian_16(n);
    return n;
}

uint32_t tr_read_u32le(const uint8_t* data)
{
    uint32_t n = *(uint32_t*)&data[0];
    if(tr_is_BE() == 1) n = tr_swap_endian_32(n);
    return n;
}

uint64_t tr_read_u64le(const uint8_t* data)
{
    uint64_t n = *(uint64_t*)&data[0];
    if(tr_is_BE() == 1) n = tr_swap_endian_64(n);
    return n;
}

float tr_read_f32le(const uint8_t* data)
{
    float n = *(float*)&data[0];
    if(tr_is_BE() == 1) n = tr_swap_endian_32(n);
    return n;
}

double tr_read_f64le(const uint8_t* data)
{
    double n = *(double*)&data[0];
    if(tr_is_BE() == 1) n = tr_swap_endian_64(n);
    return n;
}

/*
    Readers for big endian
*/
uint16_t tr_read_u16be(const uint8_t* data)
{
    uint16_t n = *(uint16_t*)&data[0];
    if(tr_is_BE() == 0) n = tr_swap_endian_16(n);
    return n;
}

uint32_t tr_read_u32be(const uint8_t* data)
{
    uint32_t n = *(uint32_t*)&data[0];
    if(tr_is_BE() == 0) n = tr_swap_endian_32(n);
    return n;
}

uint64_t tr_read_u64be(const uint8_t* data)
{
    uint64_t n = *(uint64_t*)&data[0];
    if(tr_is_BE() == 0) n = tr_swap_endian_64(n);
    return n;
}

float tr_read_f32be(const uint8_t* data)
{
    float n = *(float*)&data[0];
    if(tr_is_BE() == 0) n = tr_swap_endian_32(n);
    return n;
}

double tr_read_f64be(const uint8_t* data)
{
    double n = *(double*)&data[0];
    if(tr_is_BE() == 0) n = tr_swap_endian_64(n);
    return n;
}
