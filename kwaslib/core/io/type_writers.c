#include <string.h>
#include <stdlib.h>

#include "type_writers.h"

#include "../cpu/endianness.h"

/*
    Writers for any endianness
*/
void tw_write_u8(const uint8_t data, uint8_t* out)
{
	out[0] = data;
}

void tw_write_array(const uint8_t* data, const uint64_t len, uint8_t* out)
{
	memcpy(out, data, len);
}

/*
    Writers for host endianness.
    Doesn't check for endianness, just writes data.
*/
void tw_write_u16(const uint16_t data, uint8_t* out)
{
	tw_write_array((const uint8_t*)&data, 2, out);
}

void tw_write_u32(const uint32_t data, uint8_t* out)
{
	tw_write_array((const uint8_t*)&data, 4, out);
}

void tw_write_u64(const uint64_t data, uint8_t* out)
{
	tw_write_array((const uint8_t*)&data, 8, out);
}

void tw_write_f32(const float data, uint8_t* out)
{
	tw_write_array((const uint8_t*)&data, 4, out);
}

void tw_write_f64(const double data, uint8_t* out)
{
	tw_write_array((const uint8_t*)&data, 8, out);
}

/*
    Writers for little endian
*/
void tw_write_u16le(const uint16_t data, uint8_t* out)
{
	uint16_t n = data;
	if(ed_is_BE() == 1) n = ed_swap_endian_16(n);
	tw_write_array((const uint8_t*)&n, 2, out);
}

void tw_write_u32le(const uint32_t data, uint8_t* out)
{
	uint32_t n = data;
	if(ed_is_BE() == 1) n = ed_swap_endian_32(n);
	tw_write_array((const uint8_t*)&n, 4, out);
}

void tw_write_u64le(const uint64_t data, uint8_t* out)
{
	uint64_t n = data;
	if(ed_is_BE() == 1) n = ed_swap_endian_64(n);
	tw_write_array((const uint8_t*)&n, 8, out);
}

void tw_write_f32le(const float data, uint8_t* out)
{
	uint32_t n = *(uint32_t*)&data;
	if(ed_is_BE() == 1) n = ed_swap_endian_32(n);
	const float x = *(float*)&n;
	tw_write_array((const uint8_t*)&x, 4, out);
}

void tw_write_f64le(const double data, uint8_t* out)
{
	uint64_t n = *(uint64_t*)&data;
	if(ed_is_BE() == 1) n = ed_swap_endian_64(n);
	const double x = *(double*)&n;
	tw_write_array((const uint8_t*)&x, 8, out);
}

/*
    Writers for big endian
*/
void tw_write_u16be(const uint16_t data, uint8_t* out)
{
	uint16_t n = data;
	if(ed_is_BE() == 0) n = ed_swap_endian_16(n);
	tw_write_array((const uint8_t*)&n, 2, out);
}

void tw_write_u32be(const uint32_t data, uint8_t* out)
{
	uint32_t n = data;
	if(ed_is_BE() == 0) n = ed_swap_endian_32(n);
	tw_write_array((const uint8_t*)&n, 4, out);
}

void tw_write_u64be(const uint64_t data, uint8_t* out)
{
	uint64_t n = data;
	if(ed_is_BE() == 0) n = ed_swap_endian_64(n);
	tw_write_array((const uint8_t*)&n, 8, out);
}

void tw_write_f32be(const float data, uint8_t* out)
{
	uint32_t n = *(uint32_t*)&data;
	if(ed_is_BE() == 0) n = ed_swap_endian_32(n);
	const float x = *(float*)&n;
	tw_write_array((const uint8_t*)&x, 4, out);
}

void tw_write_f64be(const double data, uint8_t* out)
{
	uint64_t n = *(uint64_t*)&data;
	if(ed_is_BE() == 0) n = ed_swap_endian_64(n);
	const double x = *(double*)&n;
	tw_write_array((const uint8_t*)&x, 8, out);
}
