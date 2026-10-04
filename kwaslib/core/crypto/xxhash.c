#include "xxhash.h"

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/math/cirshift.h>

static const uint32_t PRIME32_1 = 0x9E3779B1;
static const uint32_t PRIME32_2 = 0x85EBCA77;
static const uint32_t PRIME32_3 = 0xC2B2AE3D;
static const uint32_t PRIME32_4 = 0x27D4EB2F;
static const uint32_t PRIME32_5 = 0x165667B1;

static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87;
static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4F;
static const uint64_t PRIME64_3 = 0x165667B19E3779F9;
static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63;
static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5;

static const uint64_t PRIME_MX1 = 0x165667919E3779F9;
static const uint64_t PRIME_MX2 = 0x9FB21C651E98DF25;

/*
    Internal utils
*/
static uint64_t xxhash_64_round(uint64_t acc, const uint64_t lane)
{
    acc += lane * PRIME64_2;
    acc =  cirshift_l64(acc, 31);
    return acc * PRIME64_1;
}

static uint64_t xxhash_64_merge_accum(uint64_t acc, const uint64_t accn)
{
    acc ^= xxhash_64_round(0, accn);
    acc *= PRIME64_1;
    return acc + PRIME64_4;
}


/*
    Implementation
*/

uint32_t xxhash_32_seed(uint8_t* data, const uint64_t data_size, const uint32_t seed)
{
    uint32_t acc[4] = {0};
    uint8_t* data_ptr = &data[0];

    /* Step 1 */
    if(data_size >= XXHASH_32_STRIPE_SIZE)
    {
        acc[0] = seed + PRIME32_1 + PRIME32_2;
        acc[1] = seed + PRIME32_2;
        acc[2] = seed + 0;
        acc[3] = seed - PRIME32_1;
    }
    else /* Less than 16 bytes */
    {
        acc[0] = seed + PRIME32_5;
        goto e32_step4;
    }
    
    /* Step 2 */
    for(uint64_t i = 0; i < data_size; i += XXHASH_32_STRIPE_SIZE)
    {
        if((data_size - i) < XXHASH_32_STRIPE_SIZE)
            break;
        
        for(uint32_t j = 0; j != 4; ++j)
        {
            acc[j] += tr_read_u32le(data_ptr) * PRIME32_2;
            acc[j] =  cirshift_l32(acc[j], 13);
            acc[j] *= PRIME32_1;
            data_ptr += XXHASH_32_LANE_SIZE;
        }
    }
    
    /* Step 3 */
    const uint32_t acc_buf = cirshift_l32(acc[0], 1) + cirshift_l32(acc[1], 7)
                           + cirshift_l32(acc[2], 12) + cirshift_l32(acc[3], 18);
    acc[0] = acc_buf;
    
    /* Step 4 */
e32_step4:
    acc[0] += (uint32_t)(data_size&0xFFFFFFFF);
    
    /* Step 5 */
    uint32_t remaining = data_size - (data_ptr - data);
    
    while(remaining >= 4)
    {
        const uint32_t lane = tr_read_u32le(data_ptr);
        acc[0] = acc[0] + lane * PRIME32_3;
        acc[0] = cirshift_l32(acc[0], 17) * PRIME32_4;
        data_ptr += 4;
        remaining -= 4;
    }
    
    while(remaining >= 1)
    {
        const uint8_t lane = data_ptr[0];
        acc[0] = acc[0] + lane * PRIME32_5;
        acc[0] = cirshift_l32(acc[0], 11) * PRIME32_1;
        data_ptr += 1;
        remaining -= 1;
    }
    
    /* Step 6 */
    acc[0] ^= (acc[0] >> 15);
    acc[0] *= PRIME32_2;
    acc[0] ^= (acc[0] >> 13);
    acc[0] *= PRIME32_3;
    acc[0] ^= (acc[0] >> 16);
    
    /* Step 7 */
    return acc[0];
}

uint64_t xxhash_64_seed(uint8_t* data, const uint64_t data_size, const uint64_t seed)
{
    uint64_t acc[4] = {0};
    uint8_t* data_ptr = &data[0];

    /* Step 1 */
    if(data_size >= XXHASH_64_STRIPE_SIZE)
    {
        acc[0] = seed + PRIME64_1 + PRIME64_2;
        acc[1] = seed + PRIME64_2;
        acc[2] = seed + 0;
        acc[3] = seed - PRIME64_1;
    }
    else /* Less than 32 bytes */
    {
        acc[0] = seed + PRIME64_5;
        goto e64_step4;
    }
    
    /* Step 2 */
    for(uint64_t i = 0; i < data_size; i += XXHASH_64_STRIPE_SIZE)
    {
        if((data_size - i) < XXHASH_64_STRIPE_SIZE)
            break;
        
        for(uint32_t j = 0; j != 4; ++j)
        {
            acc[j] = xxhash_64_round(acc[j], tr_read_u64le(data_ptr));
            data_ptr += XXHASH_64_LANE_SIZE;
        }
    }
    
    /* Step 3 */
    uint64_t acc_buf = cirshift_l64(acc[0], 1) + cirshift_l64(acc[1], 7)
                     + cirshift_l64(acc[2], 12) + cirshift_l64(acc[3], 18);
    
    acc_buf = xxhash_64_merge_accum(acc_buf, acc[0]);
    acc_buf = xxhash_64_merge_accum(acc_buf, acc[1]);
    acc_buf = xxhash_64_merge_accum(acc_buf, acc[2]);
    acc_buf = xxhash_64_merge_accum(acc_buf, acc[3]);
    
    acc[0] = acc_buf;
    
    /* Step 4 */
e64_step4:
    acc[0] += data_size;
    
    /* Step 5 */
    uint64_t remaining = data_size - (data_ptr - data);
    
    while(remaining >= 8)
    {
        const uint64_t lane = tr_read_u64le(data_ptr);
        acc[0] ^= xxhash_64_round(0, lane);
        acc[0] =  cirshift_l64(acc[0], 27) * PRIME64_1;
        acc[0] += PRIME64_4;
        data_ptr += 8;
        remaining -= 8;
    }

    while(remaining >= 4)
    {
        const uint32_t lane = tr_read_u32le(data_ptr);
        acc[0] ^= lane * PRIME64_1;
        acc[0] =  cirshift_l64(acc[0], 23) * PRIME64_2;
        acc[0] += PRIME64_3;
        data_ptr += 4;
        remaining -= 4;
    }
    
    while(remaining >= 1)
    {
        const uint8_t lane = data_ptr[0];
        acc[0] ^= lane * PRIME64_5;
        acc[0] =  cirshift_l64(acc[0], 11) * PRIME64_1;
        data_ptr += 1;
        remaining -= 1;
    }
    
    /* Step 6 */
    acc[0] ^= (acc[0] >> 33);
    acc[0] *= PRIME64_2;
    acc[0] ^= (acc[0] >> 29);
    acc[0] *= PRIME64_3;
    acc[0] ^= (acc[0] >> 32);
    
    /* Step 7 */
    return acc[0];
}