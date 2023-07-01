#include "boundary.h"

uint64_t bound_calc_leftover(const uint64_t block_size, const uint64_t value)
{
	return block_size - (value%block_size);
}