#pragma once

#include <stdint.h>

typedef uint16_t float16_t;

uint32_t half_to_float( uint16_t h );
uint16_t half_from_float( uint32_t f );
uint16_t half_add( uint16_t arg0, uint16_t arg1 );
uint16_t half_mul( uint16_t arg0, uint16_t arg1 );

static inline uint16_t 
half_sub( uint16_t ha, uint16_t hb ) 
{
  // (a-b) is the same as (a+(-b))
  return half_add( ha, hb ^ 0x8000 );
}

static inline float half_to_float32(uint16_t h)
{
	const uint32_t f32i = half_to_float(h);
	const float f32 = *(float*)&f32i;
	return f32;
}

static inline uint16_t half_from_float32(float f)
{
	return half_from_float(*(uint32_t*)&f);
}
