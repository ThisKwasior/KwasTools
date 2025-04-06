#pragma once

/*
    Hash table used for quick lookup of files with CRC32 hash.
    
    Structure contains, in order:
        - A header (DAT_HASHTABLE_HEADER)
        - 3 sections:
            - 16bit indices to internal CRC32 hash array (bucket)
            - CRC32 hash array
            - 16bit indices pointing to files in the DAT header
    
    Size of hash array and file indices array
    is equal to amount of files in the DAT header.
    
    Size of bucket has to be calculated
    from prehash shift in hashtable header:
        uint8_t bucket_size = (uint8_t)(1<<(31 - header.prehash_shift));
*/

/*
    == Using the hash table / Getting a file index from input hash
    
    Let's say we have a CRC32 hash and we want to find its index.
    First, we need to calculate the index to get an index from the bucket:
        uint16_t index_in_bucket = (hash >> header.prehash_shift);
        uint16_t index_from_bucket = bucket[index_in_bucket];
        
    Not every bucket value is valid.
    If the index_from_bucket is equal to 0xFFFF, hash does not exist in the hashmap.
    Same goes if the index_from_bucket is greater than (files_amount-1).

    After that, we have the initial index in hash array to start the search.
    We search as long as the index is the same as index_in_bucket.
    
        uint32_t* current_hash = &hash_array[index_from_bucket];
        uint32_t it = index_from_bucket; 
        while((*current_hash >> header.prehash_shift) == index_in_bucket)
        {
            if((*current_hash) == hash) return file_indices[it];
            it += 1;
            current_hash += 1;
            if(it > (files_amount-1)) return -1;
        }
*/

/*
    == Creating the hash table
    
    First and foremost, we need to calculate the prehash_shift:
        	uint8_t shift = 31 - bit_length(file_count);
            if(shift < 24) return 24;
            return shift;
    
    DATs with file counts greater than 256 still report
    the shift being 24, even though it should be 23
    and go lower with each power of 2.
    
    Now, we gotta calculate the hash array with filenames of files.
    Before throwing a filename at CRC32, we need to convert it to lowercase:
        pl1013_AE01_0_SEQ.bxm => pl1013_ae01_0_seq.bxm
    
    After we get the hash, we need to perform AND boolean operation:
        hash = hash&0x7FFFFFFF;
    
    And that's basically it for hash array.
    File indices are paired with the hash array, so
        hash[0] will have file_index[0] == 0
        hash[1] will have file_index[1] == 1
        etc
        
    At the end we need to sort both arrays with the key being:
        bucket_index = hash >> header.prehash_shift
        
    So for example, we have MGRR's pl1013.dat, and this is its bucket index arrays:
        uint8_t before_sort[] = {0, 0, 1, 7, 4, 2, 1, 5, 2, 1, 5, 6, 3, 2};
        uint8_t after_sort[]  = {0, 0, 1, 1, 1, 2, 2, 2, 3, 4, 5, 5, 6, 7};

    With that done, it's time for the bucket.
        uint8_t bucket_size = (uint8_t)(1<<(31 - header.prehash_shift));
        uint16_t bucket[bucket_size] = {0xFFFF};
        
        for(uint32_t i = 0; i != file_count; ++i)
        {
            if(bucket[after_sort[i]] == 0xFFFF)
            {
                bucket[after_sort[i]] = i;
            }
        }
*/

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/cvector.h>

typedef struct DAT_HASHTABLE_HEADER DAT_HASHTABLE_HEADER;
typedef struct DAT_HASH_ENTRY DAT_HASH_ENTRY;
typedef struct DAT_HASHTABLE DAT_HASHTABLE;

#include "dat.h"

struct DAT_HASHTABLE_HEADER
{
    uint32_t prehash_shift;
    uint32_t bucket_offset; /* Constant 0x10 */
    uint32_t hashes_offset;
    uint32_t indices_offset;
};

struct DAT_HASH_ENTRY
{
	uint32_t hash;
	uint16_t file_index;
	uint16_t bucket_index;
};

struct DAT_HASHTABLE
{
	DAT_HASHTABLE_HEADER header;
    CVEC bucket;
    CVEC entries;
};

DAT_HASHTABLE* dat_hash_read_table(FU_FILE* data, const uint32_t file_count, const uint8_t endian);

DAT_HASHTABLE* dat_hash_create_table(DAT_FILE* dat);

FU_FILE* dat_hash_to_fu_file(DAT_HASHTABLE* hashtable, const uint8_t endian);

DAT_HASH_ENTRY* dat_hash_get_entry_by_id(CVEC entries, const uint64_t id);

DAT_HASHTABLE* dat_hash_destroy(DAT_HASHTABLE* hashtable);

/* Helper */
const uint8_t dat_hash_bit_length(const uint64_t value);
const uint8_t dat_hash_calc_prehash_shift(const uint32_t file_count);
const uint8_t dat_hash_calc_bucket_size(const uint8_t prehash_shift);
const uint32_t dat_hash_crc_from_name(const uint8_t* name, const uint64_t length);
void dat_hash_sort_entries_by_bucket_id(CVEC entries);