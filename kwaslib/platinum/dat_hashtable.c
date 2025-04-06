#include "dat_hashtable.h"

#include <stdlib.h>
#include <ctype.h>

#include <kwaslib/core/crypto/crc32.h>

DAT_HASHTABLE* dat_hash_read_table(FU_FILE* data, const uint32_t file_count, const uint8_t endian)
{
    const uint64_t data_pos = fu_tell(data);
    
    DAT_HASHTABLE* ht = (DAT_HASHTABLE*)calloc(1, sizeof(DAT_HASHTABLE));
    
    /* Reading the header */
    ht->header.prehash_shift = fu_read_u32(data, NULL, endian);
    ht->header.bucket_offset = fu_read_u32(data, NULL, endian);
    ht->header.hashes_offset = fu_read_u32(data, NULL, endian);
    ht->header.indices_offset = fu_read_u32(data, NULL, endian);
    
    /* Reading the bucket */
    fu_seek(data, data_pos + ht->header.bucket_offset, SEEK_SET);
    
    const uint8_t bucket_size = dat_hash_calc_bucket_size(ht->header.prehash_shift);
    ht->bucket = cvec_create(sizeof(uint16_t));
    
    for(uint8_t i = 0; i != bucket_size; ++i)
    {
        const uint16_t bucket_value = fu_read_u16(data, NULL, endian);
        cvec_push_back(ht->bucket, (void*)&bucket_value);
    }
    
    /* Reading hashes */
    fu_seek(data, data_pos + ht->header.hashes_offset, SEEK_SET);

    ht->entries = cvec_create(sizeof(DAT_HASH_ENTRY));
    cvec_resize(ht->entries, file_count);
    
    for(uint32_t i = 0; i != file_count; ++i)
    {
        DAT_HASH_ENTRY* entry = cvec_at(ht->entries, i);
        entry->hash = fu_read_u32(data, NULL, endian);
    }
    
    /* Reading file indices */
    fu_seek(data, data_pos + ht->header.indices_offset, SEEK_SET);
    
    for(uint32_t i = 0; i != file_count; ++i)
    {
        DAT_HASH_ENTRY* entry = cvec_at(ht->entries, i);
        entry->file_index = fu_read_u16(data, NULL, endian);
    }
    
    return ht;
}

DAT_HASHTABLE* dat_hash_create_table(DAT_FILE* dat)
{
    DAT_HASHTABLE* ht = (DAT_HASHTABLE*)calloc(1, sizeof(DAT_HASHTABLE));
    
    ht->header.prehash_shift = dat_hash_calc_prehash_shift(dat->header.file_count);
    
    /* Calculating filename hashes and indices */
    ht->entries = cvec_create(sizeof(DAT_HASH_ENTRY));
    cvec_resize(ht->entries, dat->header.file_count);
    
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        DAT_FILE_ENTRY* dat_entry = dat_get_entry_by_id(dat->entries, i);
        DAT_HASH_ENTRY* hash_entry = dat_hash_get_entry_by_id(ht->entries, i);
        
        hash_entry->hash = dat_hash_crc_from_name((uint8_t*)dat_entry->name->ptr, dat_entry->name->size);
        hash_entry->file_index = i;
        hash_entry->bucket_index = hash_entry->hash >> ht->header.prehash_shift;
    }
    
    /* Sorting entries by bucket index */
    dat_hash_sort_entries_by_bucket_id(ht->entries);
    
    /* Bucket */
    ht->bucket = cvec_create(sizeof(uint16_t));
    cvec_resize(ht->bucket, dat_hash_calc_bucket_size(ht->header.prehash_shift));
    memset(ht->bucket->data, 0xFF, ht->bucket->elem_size*ht->bucket->size);
    
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        DAT_HASH_ENTRY* hash_entry = dat_hash_get_entry_by_id(ht->entries, i);
        uint16_t* bucket_value = (uint16_t*)cvec_at(ht->bucket, hash_entry->bucket_index);
        
        if((*bucket_value) == 0xFFFF)
        {
            (*bucket_value) = i;
        }
    }
    
    /* With everything done, let's prepare offsets */
    ht->header.bucket_offset = 0x10; /* Never changes */
    ht->header.hashes_offset = ht->header.bucket_offset + cvec_size(ht->bucket)*2;
    ht->header.indices_offset = ht->header.hashes_offset + dat->header.file_count*4;
    
    return ht;
}

FU_FILE* dat_hash_to_fu_file(DAT_HASHTABLE* hashtable, const uint8_t endian)
{
    FU_FILE* file = fu_alloc_file();
    fu_create_mem_file(file);
    
    /* Writing header */
    fu_write_u32(file, hashtable->header.prehash_shift, endian);
    fu_write_u32(file, hashtable->header.bucket_offset, endian);
    fu_write_u32(file, hashtable->header.hashes_offset, endian);
    fu_write_u32(file, hashtable->header.indices_offset, endian);
    
    /* Writing bucket */
    fu_change_buf_size(file, hashtable->header.bucket_offset);
    fu_seek(file, 0, SEEK_END);
    
    for(uint32_t i = 0; i != cvec_size(hashtable->bucket); ++i)
    {
        const uint16_t bucket_value = *(uint16_t*)cvec_at(hashtable->bucket, i);
        fu_write_u16(file, bucket_value, endian);
    }
    
    /* Writing hashes */
    fu_change_buf_size(file, hashtable->header.hashes_offset);
    fu_seek(file, 0, SEEK_END);
    
    for(uint32_t i = 0; i != cvec_size(hashtable->entries); ++i)
    {
        DAT_HASH_ENTRY* entry = (DAT_HASH_ENTRY*)cvec_at(hashtable->entries, i);
        fu_write_u32(file, entry->hash, endian);
    }
    
    /* Writing file indices */
    fu_change_buf_size(file, hashtable->header.indices_offset);
    fu_seek(file, 0, SEEK_END);
    
    for(uint32_t i = 0; i != cvec_size(hashtable->entries); ++i)
    {
        DAT_HASH_ENTRY* entry = (DAT_HASH_ENTRY*)cvec_at(hashtable->entries, i);
        fu_write_u16(file, entry->file_index, endian);
    }
    
    return file;
}

DAT_HASH_ENTRY* dat_hash_get_entry_by_id(CVEC entries, const uint64_t id)
{
    return (DAT_HASH_ENTRY*)cvec_at(entries, id);
}

DAT_HASHTABLE* dat_hash_destroy(DAT_HASHTABLE* hashtable)
{
    if(hashtable)
    {
        cvec_destroy(hashtable->bucket);
        cvec_destroy(hashtable->entries);
        free(hashtable);
    }
    
    return NULL;
}

/* Helper */

const uint8_t dat_hash_bit_length(const uint64_t value)
{
	if(value == 1)
    {
        return 1;
    }
	
	uint8_t last_bit_pos = 0;
	
	for(uint8_t i = 0; i != 64; ++i)
	{
		const uint8_t bit = (value>>i)&1;

		if(bit)
        {
            last_bit_pos = i;
        }
	}
	
	return last_bit_pos;
}

const uint8_t dat_hash_calc_prehash_shift(const uint32_t file_count)
{
	const uint8_t shift = 31 - dat_hash_bit_length(file_count);
	
	/* 
		DATs with file counts greater than 256 
		still report the shift being 24, even though it
		should be 23 and go lower with each power of 2.
	*/
	if(shift < 24) return 24;
	
	return shift;
}

const uint8_t dat_hash_calc_bucket_size(const uint8_t prehash_shift)
{
	return (uint8_t)(1<<(31 - prehash_shift));
}

const uint32_t dat_hash_crc_from_name(const uint8_t* name, const uint64_t length)
{
	uint8_t* lowercase = (uint8_t*)calloc(1, length);
	
	for(uint32_t i = 0; i != length; ++i)
	{
		lowercase[i] = tolower(name[i]);
	}
	
	const uint32_t hash = crc32_encode(lowercase, length)&0x7FFFFFFF;
	
	free(lowercase);
	
	return hash;
}

void dat_hash_sort_entries_by_bucket_id(CVEC entries)
{
    for(uint32_t i = 0; i != cvec_size(entries)-1; ++i)
    {
        for(uint32_t j = 0; j != cvec_size(entries)-1; ++j)
        {
            DAT_HASH_ENTRY* f = dat_hash_get_entry_by_id(entries, j);
            DAT_HASH_ENTRY* s = dat_hash_get_entry_by_id(entries, j+1);
            
            if(f->bucket_index > s->bucket_index)
            {
                cvec_swap_elements_pos(entries, j, j+1);
            }
        }
    }
}