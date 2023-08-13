#include "endianness.h"

/*
    Check endianness
*/
uint8_t ed_is_BE()
{
    const uint16_t i = 0x0102;
    const uint8_t* ia = (const uint8_t*)&i;
    if(ia[0] == 0x01) return 1;
    return 0;
}

/*
    Changes endianness of the number
*/
uint16_t ed_swap_endian_16(const uint16_t n)
{
	return (n << 8) | (n >> 8);
}

uint32_t ed_swap_endian_32(const uint32_t n)
{
	return (n >> 24) | 
		   ((n & 0xFF0000) >> 8) |
		   ((n & 0xFF00) << 8) |
		   (n << 24);
}

uint64_t ed_swap_endian_64(const uint64_t n)
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