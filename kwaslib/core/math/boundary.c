#include "boundary.h"

uint64_t bound_calc_leftover(const uint64_t block_size, const uint64_t value)
{
    uint64_t calc = block_size - (value%block_size);
	
    if(calc == block_size) return 0;
    
    return calc;
}