#pragma once

#include <stdint.h>

static inline const uint64_t bound_calc_leftover(const uint64_t block_size, const uint64_t value)
{
    if(block_size == 0) return 0;
    
    uint64_t calc = block_size - (value%block_size);
	
    if(calc == block_size) return 0;
    
    return calc;
}