#pragma once

#include <stdint.h>

#define ED_ENDIAN_BIG       (uint8_t)(1)
#define ED_ENDIAN_LITTLE    (uint8_t)(0)

/*
    Check endianness
*/
static inline const uint8_t ed_is_BE()
{
#if defined(KWASLIB_BIG_ENDIAN)
    return ED_ENDIAN_BIG;
#elif defined(KWASLIB_LITTLE_ENDIAN)
    return ED_ENDIAN_LITTLE;
#else
    #warning "No endianness specified with KWASLIB_BIG_ENDIAN or KWASLIB_LITTLE_ENDIAN. Using slower code path."
    const uint16_t i = 0x0102;
    const uint8_t* ia = (const uint8_t*)&i;
    if(ia[0] == 0x01) return ED_ENDIAN_BIG;
    return ED_ENDIAN_LITTLE;
#endif
}

/*
    Changes endianness of the number
*/
static inline const uint16_t ed_swap_endian_16(const uint16_t n)
{
	return (n << 8) | (n >> 8);
}

static inline const uint32_t ed_swap_endian_32(const uint32_t n)
{
	return (n >> 24) | 
		   ((n & 0xFF0000) >> 8) |
		   ((n & 0xFF00) << 8) |
		   (n << 24);
}

static inline const uint64_t ed_swap_endian_64(const uint64_t n)
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