#include "dat.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <kwaslib/core/crypto/crc32.h>
#include <kwaslib/core/math/boundary.h>

DAT_FILE* dat_parse(FU_FILE* file, const uint8_t fu_endian)
{
	DAT_FILE* dat = (DAT_FILE*)calloc(1, sizeof(DAT_FILE));
	
	if(dat == NULL) return NULL;
	
	/* Reading magic */
	fu_read_data(file, &dat->header.magic[0], 4, NULL);
	
	/* Check if we have a DAT\0 file */
	if(strncmp(DAT_MAGIC, (const char*)&dat->header.magic[0], 4) != 0)
	{
		free(dat);
		return NULL;
	}
	
	/* Reading the rest of the header */
	dat->header.file_count = fu_read_u32(file, NULL, fu_endian);
	dat->header.positions_offset = fu_read_u32(file, NULL, fu_endian);
	dat->header.extensions_offset = fu_read_u32(file, NULL, fu_endian);
	dat->header.names_offset = fu_read_u32(file, NULL, fu_endian);
	dat->header.sizes_offset = fu_read_u32(file, NULL, fu_endian);
	dat->header.hashes_offset = fu_read_u32(file, NULL, fu_endian);
	dat->header.unk1C = fu_read_u32(file, NULL, fu_endian);
	
	/* Reading file entries */
	fu_seek(file, dat->header.names_offset, FU_SEEK_SET);
	dat->entry_name_size = fu_read_u32(file, NULL, fu_endian);
	
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		uint32_t position = 0;
		uint8_t extension[4] = {0};
		uint8_t* name = NULL;
		uint32_t size = 0;
		uint8_t* data = NULL;
		
		fu_seek(file, dat->header.positions_offset + (i*4), FU_SEEK_SET);
		position = fu_read_u32(file, NULL, fu_endian);
		
		fu_seek(file, dat->header.extensions_offset + (i*4), FU_SEEK_SET);
		fu_read_data(file, &extension[0], 4, NULL);
		
		fu_seek(file, dat->header.names_offset + 4 + (i*dat->entry_name_size), FU_SEEK_SET);
		name = (uint8_t*)calloc(1, dat->entry_name_size);
		fu_read_data(file, &name[0], dat->entry_name_size, NULL);
		
		fu_seek(file, dat->header.sizes_offset + (i*4), FU_SEEK_SET);
		size = fu_read_u32(file, NULL, fu_endian);
		
		fu_seek(file, position, FU_SEEK_SET);
		data = (uint8_t*)calloc(1, size);
		fu_read_data(file, &data[0], size, NULL);
		
		DAT_FILE_ENTRY* entry = dat_entry_from_data(position, &extension[0],
													dat->entry_name_size, name,
													size, data);
													
		dat->entries = dat_append_entry(dat->entries, entry);
		
		free(name);
		free(data);
	}
	
	/* Reading hashes */
	fu_seek(file, dat->header.hashes_offset, FU_SEEK_SET);
	dat->hashes.prehash_shift = fu_read_u32(file, NULL, fu_endian);
	dat->hashes.bucket_offset = fu_read_u32(file, NULL, fu_endian);
	dat->hashes.hashes_offset = fu_read_u32(file, NULL, fu_endian);
	dat->hashes.indices_offset = fu_read_u32(file, NULL, fu_endian);
	dat->hashes.bucket_size = dat_calc_bucket_size(dat->hashes.prehash_shift);
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.bucket_offset, FU_SEEK_SET);
	dat->hashes.bucket = (uint16_t*)calloc(dat->hashes.bucket_size, sizeof(uint16_t));
	for(uint32_t i = 0; i != dat->hashes.bucket_size; ++i)
	{
		dat->hashes.bucket[i] = fu_read_u16(file, NULL, fu_endian);
	}
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.hashes_offset, FU_SEEK_SET);
	dat->hashes.hashes = (DAT_HASH*)calloc(dat->header.file_count, sizeof(DAT_HASH));
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		dat->hashes.hashes[i].index = i;
		dat->hashes.hashes[i].hash = fu_read_u32(file, NULL, fu_endian);
	}
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.indices_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		dat->hashes.hashes[i].pos = fu_read_u16(file, NULL, fu_endian);
	}
	
	return dat;
}

FU_FILE* dat_save_to_fu_file(DAT_FILE* dat, const uint8_t fu_endian)
{
	FU_FILE* file = fu_alloc_file();
	fu_create_mem_file(file);
	
	fu_write_data(file, &dat->header.magic[0], 4);
	fu_write_u32(file, dat->header.file_count, fu_endian);
	fu_write_u32(file, dat->header.positions_offset, fu_endian);
	fu_write_u32(file, dat->header.extensions_offset, fu_endian);
	fu_write_u32(file, dat->header.names_offset, fu_endian);
	fu_write_u32(file, dat->header.sizes_offset, fu_endian);
	fu_write_u32(file, dat->header.hashes_offset, fu_endian);
	fu_write_u32(file, dat->header.unk1C, fu_endian);
	
	fu_change_buf_size(file, dat->header.hashes_offset);
	
	/* Write positions */
	fu_seek(file, dat->header.positions_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		fu_write_u32(file, entry->position, fu_endian);
	}
	
	/* Write extensions */
	fu_seek(file, dat->header.extensions_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		fu_write_data(file, &entry->extension[0], 4);
	}
	
	/* Write names */
	fu_seek(file, dat->header.names_offset, FU_SEEK_SET);
	fu_write_u32(file, dat->entry_name_size, fu_endian);
	
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		fu_seek(file, dat->header.names_offset + 4 + (i*dat->entry_name_size), FU_SEEK_SET);
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		fu_write_data(file, entry->name, strlen((const char*)entry->name));
	}
	
	/* Write sizes */
	fu_seek(file, dat->header.sizes_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		fu_write_u32(file, entry->size, fu_endian);
	}
	
	/* Write hashes */
	fu_seek(file, dat->header.hashes_offset, FU_SEEK_SET);
	fu_write_u32(file, dat->hashes.prehash_shift, fu_endian);
	fu_write_u32(file, dat->hashes.bucket_offset, fu_endian);
	fu_write_u32(file, dat->hashes.hashes_offset, fu_endian);
	fu_write_u32(file, dat->hashes.indices_offset, fu_endian);
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.bucket_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->hashes.bucket_size; ++i)
		fu_write_u16(file, dat->hashes.bucket[i], fu_endian);
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.hashes_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
		fu_write_u32(file, dat->hashes.hashes[i].hash, fu_endian);
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.hashes_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
		fu_write_u32(file, dat->hashes.hashes[dat->hashes.order[i]].hash, fu_endian);
	
	fu_seek(file, dat->header.hashes_offset + dat->hashes.indices_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
		fu_write_u16(file, dat->hashes.hashes[dat->hashes.order[i]].index, fu_endian);
	
	/* Writing file data */
	DAT_FILE_ENTRY* last_entry = dat_get_entry_node(dat->entries, -1);
	fu_change_buf_size(file, last_entry->position);
	
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		fu_seek(file, entry->position, FU_SEEK_SET);
		fu_write_data(file, entry->data, entry->size);
	}
	
	return file;
}

void dat_update(DAT_FILE* dat, const uint32_t alignment)
{
	if(dat->entries == NULL) return;
	
	dat_free_hashes(&dat->hashes);
	
	dat->hashes = dat_generate_hashes(dat->entries);
	dat->entry_name_size = dat_get_max_name_size(dat->entries);
	dat->header = dat_generate_header(dat->entries, &dat->hashes, dat->entry_name_size);
	dat_generate_positions(&dat->header, &dat->hashes, dat->entries, alignment);
}

DAT_FILE_ENTRY* dat_entry_from_data(const uint32_t position,
									const uint8_t* extension,
									const uint32_t name_size,
									const uint8_t* name,
									const uint32_t size,
									const uint8_t* data)
{
	DAT_FILE_ENTRY* entry = (DAT_FILE_ENTRY*)calloc(1, sizeof(DAT_FILE_ENTRY));
	
	entry->position = position;
	
	memcpy(&entry->extension[0], extension, 4);
	
	entry->name = (uint8_t*)calloc(1, name_size+1); /* NULL terminator */
	memcpy(&entry->name[0], name, name_size);
	
	entry->size = size;
	
	entry->data = (uint8_t*)calloc(1, size);
	memcpy(&entry->data[0], data, size);
	
	return entry;
}

DAT_HEADER dat_generate_header(DBL_LIST_NODE* entries,
							   DAT_HASHES* hashes,
							   const uint32_t entry_name_size)
{
	DAT_HEADER h = {0};
	
	memcpy(&h.magic[0], DAT_MAGIC, 4);
	h.file_count = dbl_list_count(entries);
	
	uint32_t cur_pos = 0x20;
	
	h.positions_offset = cur_pos;
	
	cur_pos += (4*h.file_count);
	h.extensions_offset = cur_pos;
	
	cur_pos += (4*h.file_count);
	h.names_offset = cur_pos;
	
	/* 
		Sections in the header are aligned to 4 bytes.
		We can't be certain the names section will always be aligned
		so we need to add padding.
	*/
	cur_pos += 4 + (entry_name_size*h.file_count);
	cur_pos += bound_calc_leftover(4, cur_pos);
	h.sizes_offset = cur_pos;
	
	cur_pos += (4*h.file_count);
	h.hashes_offset = cur_pos;
	
	return h;
}

DAT_HASHES dat_generate_hashes(DBL_LIST_NODE* entries)
{
	DAT_HASHES hashes = {0};
	
	const uint32_t file_count = dbl_list_count(entries);
	hashes.prehash_shift = dat_calc_prehash_shift(file_count);
	hashes.bucket_size = dat_calc_bucket_size(hashes.prehash_shift);
	
	hashes.hashes = (DAT_HASH*)calloc(file_count, sizeof(DAT_HASH));
	hashes.order = (uint32_t*)calloc(file_count, sizeof(uint32_t));
	hashes.bucket = (uint16_t*)calloc(hashes.bucket_size, sizeof(uint16_t));
	memset(hashes.bucket, 0xFF, hashes.bucket_size*2);
	
	for(uint32_t i = 0; i != file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(entries, i);
		hashes.hashes[i].hash = dat_calc_hash_from_name(entry->name, strlen((const char*)entry->name));
		hashes.hashes[i].pos = hashes.hashes[i].hash>>hashes.prehash_shift;
		hashes.hashes[i].index = i;
	}
	
	/* Generating the order */
	uint32_t min = -1;
	uint32_t max = 0;
	
	for(uint32_t i = 0; i != file_count; ++i)
	{
		if(min > hashes.hashes[i].pos) min = hashes.hashes[i].pos;
		if(max < hashes.hashes[i].pos) max = hashes.hashes[i].pos;
	}

	uint32_t order_it = 0;
	for(uint32_t i = 0; i != file_count; ++i)
	{
		for(uint32_t j = 0; j != file_count; ++j)
		{
			if(i == hashes.hashes[j].pos)
			{
				hashes.order[order_it++] = hashes.hashes[j].index;
			}
		}
	}
	
	/* Generate bucket offsets */
	for(uint32_t i = 0; i != file_count; ++i)
	{
		if(hashes.bucket_size > hashes.hashes[i].pos)
		{
			if(hashes.bucket[hashes.hashes[hashes.order[i]].pos] == 0xFFFF)
			{
				hashes.bucket[hashes.hashes[hashes.order[i]].pos] = i;
			}
		}
	}
	
	/* Calculating offsets */
	hashes.bucket_offset = 0x10;
	hashes.hashes_offset = hashes.bucket_offset + (2*hashes.bucket_size);
	hashes.indices_offset = hashes.hashes_offset + (4*file_count);
	
	return hashes;
}

void dat_generate_positions(DAT_HEADER* header, DAT_HASHES* hashes,
							DBL_LIST_NODE* entries, const uint32_t alignment)
{
	uint32_t pos = header->hashes_offset;
	
	/* Hashes section */
	pos += 16 + (2*hashes->bucket_size) + (4*header->file_count)*2;
	
	/* First position */
	if(alignment)
		pos += bound_calc_leftover(alignment, pos);
	
	for(uint32_t i = 0; i != header->file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(entries, i);
		entry->position = pos;
		pos += entry->size;
		
		if(alignment)
		{
			const uint32_t padding = bound_calc_leftover(alignment, pos);
			pos += padding;
		}
	}
}

DBL_LIST_NODE* dat_append_entry(DBL_LIST_NODE* entries, DAT_FILE_ENTRY* entry)
{
	DBL_LIST_NODE* node = dbl_list_append(entries, entry, sizeof(DAT_FILE_ENTRY));
	
	return node;
}

DAT_FILE_ENTRY* dat_get_entry_node(DBL_LIST_NODE* entries, const uint64_t pos)
{
	return (DAT_FILE_ENTRY*)dbl_list_get_node(entries, pos)->data;
}

void dat_free_entry(DAT_FILE_ENTRY* entry)
{
	entry->position = 0;
	entry->size = 0;
	memset(entry->extension, 0, 4);
	free(entry->name);
	entry->name = NULL;
	free(entry->data);
	entry->data = NULL;
}

void dat_free_hashes(DAT_HASHES* hashes)
{
	free(hashes->bucket);
	free(hashes->hashes);
	free(hashes->order);
	memset(hashes, 0, sizeof(DAT_HASHES));
}

void dat_free(DAT_FILE* dat)
{
	DBL_LIST_NODE* node = dat->entries;
	while(node != NULL)
	{
		dat_free_entry((DAT_FILE_ENTRY*)node->data);
		node = node->next;
	}
	
	dat->entries = dbl_list_free_list(dat->entries);
	
	memset(&dat->header, 0, sizeof(DAT_HEADER));
	
	dat_free_hashes(&dat->hashes);
}

/* Helper */

const uint8_t dat_bit_length(const uint64_t value)
{
	if(value == 1) return 1;
	
	uint8_t last_bit_pos = 0;
	
	for(uint8_t i = 0; i != 64; ++i)
	{
		const uint8_t bit = (value>>i)&1;
		if(bit) last_bit_pos = i;
	}
	
	return last_bit_pos;
}

const uint8_t dat_calc_prehash_shift(const uint32_t file_count)
{
	const uint8_t shift = 31 - dat_bit_length(file_count);
	
	/* 
		DATs with file counts greater than 256 
		still report the shift being 24, even though it
		should be 23 and go lower with each power of 2.
	*/
	if(shift < 24) return 24;
	
	return shift;
}

const uint8_t dat_calc_bucket_size(const uint8_t prehash_shift)
{
	return (uint8_t)(1<<(31 - prehash_shift));
}

const uint32_t dat_get_max_name_size(DBL_LIST_NODE* entries)
{
	uint32_t len = 0;
	
	DBL_LIST_NODE* node = entries;
	while(node != NULL)
	{
		const DAT_FILE_ENTRY* entry = (DAT_FILE_ENTRY*)node->data;
		const uint32_t name_len = strlen((const char*)entry->name);
		if(name_len > len) len = name_len;
		node = node->next;
	}
	
	return len + 1;
}

const uint32_t dat_calc_hash_from_name(const uint8_t* name, const uint64_t length)
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

