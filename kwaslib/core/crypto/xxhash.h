#pragma once

/*
    xxHash family of algorithms.
    Spec taken from https://github.com/Cyan4973/xxHash/blob/release/doc/xxhash_spec.md
    
    TODO: XXH3
*/

#include <stdint.h>

#define XXHASH_32_STRIPE_SIZE       (uint64_t)(16)
#define XXHASH_32_LANE_SIZE         (uint64_t)(4)
#define XXHASH_64_STRIPE_SIZE       (uint64_t)(32)
#define XXHASH_64_LANE_SIZE         (uint64_t)(8)

uint32_t xxhash_32_seed(uint8_t* data, const uint64_t data_size, const uint32_t seed);

uint64_t xxhash_64_seed(uint8_t* data, const uint64_t data_size, const uint64_t seed);

/*
    Functions with default values for hashers
*/

static inline uint32_t xxhash_32(uint8_t* data, const uint64_t data_size)
{
    return xxhash_32_seed(data, data_size, 0);
}

static inline uint64_t xxhash_64(uint8_t* data, const uint64_t data_size)
{
    return xxhash_64_seed(data, data_size, 0);
}