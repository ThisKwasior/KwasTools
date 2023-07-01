#include "dat.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kwaslib/utils/crypto/crc32.h>
#include <kwaslib/utils/math/boundary.h>

#define DAT_BLOCK_SIZE 4096

DAT_FILE* platinum_dat_parse_dat(FU_FILE* file, const uint8_t fu_endian)
{
	DAT_FILE* dat = (DAT_FILE*)calloc(1, sizeof(DAT_FILE));
	if(dat == NULL)
	{
		printf("Could not allocate dat file structure\n");
		return NULL;
	}
	
	/* Reading the magic */
	uint64_t bytes_read = 0;
	uint8_t status = 0;
	fu_read_data(file, &dat->header.magic[0], 4, &bytes_read);
	
	if(strncmp("DAT\0", &dat->header.magic[0], 4) != 0)
	{
		printf("File is not a valid DAT file\n");
		free(dat);
		return NULL;
	}
	
	/* Reading the rest of the header */
	dat->header.files_amount = fu_read_u32(file, &status, fu_endian);
	dat->header.positions_offset = fu_read_u32(file, &status, fu_endian);
	dat->header.extensions_offset = fu_read_u32(file, &status, fu_endian);
	dat->header.names_offset = fu_read_u32(file, &status, fu_endian);
	dat->header.sizes_offset = fu_read_u32(file, &status, fu_endian);
	dat->header.hashes_offset = fu_read_u32(file, &status, fu_endian);
	dat->header.unk1C = fu_read_u32(file, &status, fu_endian);
	
	/* Allocating entries */
	dat->entries = (DAT_FILE_ENTRY*)calloc(1, dat->header.files_amount*sizeof(DAT_FILE_ENTRY));
	if(dat->entries == NULL)
	{
		printf("Could not allocate entries\n");
		free(dat);
		return NULL;
	}
	
	/* Reading the names size */
	fu_seek(file, dat->header.names_offset, FU_SEEK_SET);
	
	dat->entry_name_size = fu_read_u32(file, &status, fu_endian);
	
	/* Reading entries */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		/* Position */
		fu_seek(file, dat->header.positions_offset+(i*4), FU_SEEK_SET);
		dat->entries[i].position = fu_read_u32(file, &status, fu_endian);
		
		/* Extension */
		fu_seek(file, dat->header.extensions_offset+(i*4), FU_SEEK_SET);
		fu_read_data(file, (uint8_t*)&dat->entries[i].extension[0], 4, &bytes_read);
		
		/* Names */
		fu_seek(file, dat->header.names_offset+4+(i*dat->entry_name_size), FU_SEEK_SET);
		dat->entries[i].name = (uint8_t*)calloc(1, dat->entry_name_size);
		fu_read_data(file, dat->entries[i].name, dat->entry_name_size, &bytes_read);
		
		/* Sizes */
		fu_seek(file, dat->header.sizes_offset+(i*4), FU_SEEK_SET);
		dat->entries[i].size = fu_read_u32(file, &status, fu_endian);
		
		/* Data */
		fu_seek(file, dat->entries[i].position, FU_SEEK_SET);
		dat->entries[i].data = (uint8_t*)calloc(1, dat->entries[i].size);
		fu_read_data(file, dat->entries[i].data, dat->entries[i].size, &bytes_read);
	}
	
	/* Hash info */
	fu_seek(file, dat->header.hashes_offset, FU_SEEK_SET);
	dat->prehash_shift = fu_read_u32(file, &status, fu_endian);
	dat->bucket_offsets_offset = fu_read_u32(file, &status, fu_endian);
	dat->hashes_offset = fu_read_u32(file, &status, fu_endian);
	dat->indices_offset = fu_read_u32(file, &status, fu_endian);
	
	dat->bucket_offsets_size = (dat->hashes_offset - dat->bucket_offsets_offset)/2;
	
	/* Bucket offsets */
	fu_seek(file, dat->header.hashes_offset+dat->bucket_offsets_offset, FU_SEEK_SET);
	dat->bucket_offsets = (uint16_t*)calloc(1, dat->bucket_offsets_size*2);
	for(uint32_t i = 0; i != dat->bucket_offsets_size; ++i)
	{
		dat->bucket_offsets[i] = fu_read_u16(file, &status, fu_endian);
	}
	
	/* Indices */
	fu_seek(file, dat->header.hashes_offset+dat->indices_offset, FU_SEEK_SET);
	dat->indices = (uint16_t*)calloc(1, dat->header.files_amount*2);
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		dat->indices[i] = fu_read_u16(file, &status, fu_endian);
	}
	
	/* Hashes */
	fu_seek(file, dat->header.hashes_offset+dat->hashes_offset, FU_SEEK_SET);
	dat->hashes = (uint32_t*)calloc(1, dat->header.files_amount*4);
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		//fu_read_data(file, (uint8_t*)&dat->hashes[dat->indices[i]], 4, &bytes_read);
		dat->hashes[dat->indices[i]] = fu_read_u32(file, &status, fu_endian);
	}
	
	return dat;
}

DAT_FILE* platinum_dat_parse_directory(const char* dir)
{
	DL_DIR_LIST dirlist = {0};
	dl_parse_directory(dir, &dirlist);
	
	DAT_FILE* dat = (DAT_FILE*)calloc(1, sizeof(DAT_FILE));
	if(dat == NULL)
	{
		printf("Could not allocate dat file structure\n");
		return NULL;
	}
	
	strncpy(dat->header.magic, "DAT\0", 4);
	
	dat->header.files_amount = dirlist.file_count;
	
	for(uint32_t i = 0; i != dirlist.size; ++i)
	{
		if(dirlist.entries[i].type == DL_TYPE_FILE)
		{
			PU_STRING temp_file_path = {0};
			pu_path_to_string(&dirlist.entries[i].path, &temp_file_path);
			if(dat->entry_name_size < temp_file_path.s)
			{
				dat->entry_name_size = temp_file_path.s;
			}
			pu_free_string(&temp_file_path);
		}
	}
	
	/* For NULL terminator */
	dat->entry_name_size += 1;
	
	/* 
		Allocate and populate DAT entries 
	*/
	dat->entries = (DAT_FILE_ENTRY*)calloc(dat->header.files_amount, sizeof(DAT_FILE_ENTRY));
	
	uint32_t dir_it = 0;
	for(uint32_t i = 0; i != dirlist.size; ++i)
	{
		if(dirlist.entries[i].type == DL_TYPE_FILE)
		{
			/* Preparing file path */
			PU_STRING temp_file_path = {0};
			pu_path_to_string(&dirlist.entries[i].path, &temp_file_path);
			
			PU_STRING file_dir_path = {0};
			pu_path_to_string(&dirlist.path, &file_dir_path);
			pu_insert_char("/", 1, -1, &file_dir_path);
			pu_insert_char(temp_file_path.p, temp_file_path.s, -1, &file_dir_path);
			
			/* Open the file */
			FU_FILE temp_file = {0};
			fu_open_file(file_dir_path.p, 1, &temp_file);
			
			/* load the data to DAT entry */
			dat->entries[dir_it].size = temp_file.size;
			
			memcpy(dat->entries[dir_it].extension, dirlist.entries[i].path.ext.p, 3);
			
			dat->entries[dir_it].name = (uint8_t*)calloc(1, dat->entry_name_size);
			memcpy(dat->entries[dir_it].name, temp_file_path.p, temp_file_path.s);
			
			uint64_t bytes_read = 0;
			dat->entries[dir_it].data = (uint8_t*)calloc(1, dat->entries[dir_it].size);
			fu_read_data(&temp_file, dat->entries[dir_it].data,
						 dat->entries[dir_it].size, &bytes_read);
			
			/* Freeing strings and closing the file */
			pu_free_string(&temp_file_path);
			pu_free_string(&file_dir_path);
			fu_close(&temp_file);
			
			/* Next DAT entry */
			dir_it += 1;
		}
	}
	
	/* 
		Figure out offsets in the header
	*/
	
	/* Constant. Always after the header. */
	/* Header is 0x20 bytes.*/
	/* Positions are 32bit unsigned integers */
	dat->header.positions_offset = 0x20;
	
	/* Extensions are 3 characters long in 4 byte arrays */
	/* Position offset + positions*files amount */
	dat->header.extensions_offset = dat->header.positions_offset
									+ (4*dat->header.files_amount);
	
	/* Names have a fixed size, the longest file name + NULL terminator */
	/* The first unsigned int in names sections is the size */
	/* Extensions offset + extensions*files amount */
	dat->header.names_offset = dat->header.extensions_offset
							   + (4*dat->header.files_amount);

	/* Sizes are 32bit unsigned integers */
	dat->header.sizes_offset = dat->header.names_offset
							   + (dat->entry_name_size*dat->header.files_amount)
							   + 4;
							  
	/* Hashes is a section with its own sections */
	/* First is the 0x10 header with a unsigned 32bit shift value and 3 offsets */
	dat->header.hashes_offset = dat->header.sizes_offset
							    + (4*dat->header.files_amount);
								
	/*
		Generate hashes
	*/
	platinum_dat_gen_hash_data(dat);
	
	/*
		Figure out file positions.
		Files are aligned to 4096 byte boundaries.
	*/
	const uint32_t temp_data_pos = dat->header.hashes_offset
								   + dat->indices_offset
								   + 2*dat->header.files_amount;
	
	/*uint32_t bytes_to_pad = 4096 - (temp_data_pos%4096);*/
	uint64_t bytes_to_pad = bound_calc_leftover(DAT_BLOCK_SIZE, temp_data_pos);
	
	const uint32_t first_data_pos = temp_data_pos + bytes_to_pad;
	uint32_t cur_data_pos = first_data_pos;
	
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		dat->entries[i].position = cur_data_pos;
		
		cur_data_pos += dat->entries[i].size;
		/*bytes_to_pad = 4096 - (cur_data_pos%4096);*/
		bytes_to_pad = bound_calc_leftover(DAT_BLOCK_SIZE, cur_data_pos);
		cur_data_pos += bytes_to_pad;
	}
	
	dl_free_list(&dirlist);

	return dat;
}

FU_FILE* platinum_dat_save_to_fu_file(DAT_FILE* dat, const uint8_t fu_endian)
{
	FU_FILE* file = (FU_FILE*)calloc(1, sizeof(FU_FILE));
	fu_create_mem_file(file);
	
	//fu_write_data(file, (const uint8_t*)&dat->header, sizeof(DAT_HEADER));
	fu_write_data(file, &dat->header.magic[0], 4);
	fu_write_u32(file, dat->header.files_amount, fu_endian);
	fu_write_u32(file, dat->header.positions_offset, fu_endian);
	fu_write_u32(file, dat->header.extensions_offset, fu_endian);
	fu_write_u32(file, dat->header.names_offset, fu_endian);
	fu_write_u32(file, dat->header.sizes_offset, fu_endian);
	fu_write_u32(file, dat->header.hashes_offset, fu_endian);
	fu_write_u32(file, dat->header.unk1C, fu_endian);

	/* Write positions */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_u32(file, dat->entries[i].position, fu_endian);
	
	/* Write extensions */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_data(file, dat->entries[i].extension, 4);
	
	/* Write names */
	fu_write_u32(file, dat->entry_name_size, fu_endian);
	
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_data(file, dat->entries[i].name, dat->entry_name_size);
	
	/* Write sizes */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_u32(file, dat->entries[i].size, fu_endian);
	
	/* Write hash info */
	fu_write_u32(file, dat->prehash_shift, fu_endian);
	fu_write_u32(file, dat->bucket_offsets_offset, fu_endian);
	fu_write_u32(file, dat->hashes_offset, fu_endian);
	fu_write_u32(file, dat->indices_offset, fu_endian);

	/* Bucket offsets */
	for(uint32_t i = 0; i != dat->bucket_offsets_size; ++i)
		fu_write_u16(file, dat->bucket_offsets[i], fu_endian);
	
	/* Hashes */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_u32(file, dat->hashes[i], fu_endian);
		//fu_write_u32(file, dat->hashes[dat->indices[i]], FU_HOST_ENDIAN);
	
	/* Indices */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		fu_write_u16(file, dat->indices[i], fu_endian);
	
	/* File data */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		const int64_t padding = dat->entries[i].position - fu_tell(file);
		fu_add_to_buf_size(file, padding);
		fu_seek(file, 0, FU_SEEK_END);
		fu_write_data(file, dat->entries[i].data, dat->entries[i].size);
	}
	
	/* Pad the end so that it's aligned to 4096 bytes */
	/*const int64_t padding = 4096 - (fu_tell(&file)%4096);*/
	const int64_t padding = bound_calc_leftover(DAT_BLOCK_SIZE, fu_tell(file));
	fu_add_to_buf_size(file, padding);
	
	return file;
}

void platinum_dat_gen_hash_data(DAT_FILE* dat)
{
	dat->prehash_shift = platinum_dat_calc_prehash_shift(dat->header.files_amount);
	dat->bucket_offsets_size = 1 << (31 - dat->prehash_shift);

	dat->bucket_offsets = (uint16_t*)calloc(dat->bucket_offsets_size, 2);
	dat->hashes = (uint32_t*)calloc(dat->header.files_amount, 4);
	dat->indices = (uint16_t*)calloc(dat->header.files_amount, 2);

	memset(dat->bucket_offsets, 0xFF, dat->bucket_offsets_size*2);

	/* Hash filenames */
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		dat->hashes[i] = platinum_dat_hash_filename(dat->entries[i].name);
		dat->indices[i] = i;
	}

	/* Sort by hash nibbles */
	platinum_dat_sort_hashes(dat->hashes, dat->indices,
							 dat->header.files_amount);

	/* Generating bucket list */
	platinum_dat_gen_bucket_list(dat->header.files_amount, dat->bucket_offsets,
								 dat->hashes, dat->prehash_shift);

	/* Set offset variables */
	dat->bucket_offsets_offset = 0x10; /* Constant */
	dat->hashes_offset = dat->bucket_offsets_offset + dat->bucket_offsets_size*2;
	dat->indices_offset = dat->hashes_offset + dat->header.files_amount*4;
}

uint32_t platinum_dat_hash_filename(const uint8_t* name)
{
	return crc32_encode(name, strlen(name)) & 0x7FFFFFFF;
}

uint32_t platinum_dat_bit_count(uint32_t value)
{
	uint32_t count = 0;
	
	while(value > 0)
	{
		count += 1;
		value >>= 1;
	}
	
	return count;
}

uint32_t platinum_dat_next_pow_of_2_bits(uint32_t value)
{
	if(value == 0)
	{
		return 1;
	}
	
	/*
		MGRR has this wrong with 1 or 2 files 
		And it breaks on larget values for some reason, 1378 for example.
	*/
	if(value == 1 || value == 2)
	{
		return 2;
	}

	return platinum_dat_bit_count(value - 1);
}

uint32_t platinum_dat_calc_prehash_shift(uint32_t value)
{
	const uint32_t max_prehash = 31;
	const uint32_t calc_prehash = 32 - platinum_dat_next_pow_of_2_bits(value);
	return (max_prehash > calc_prehash) ? calc_prehash : max_prehash;
}

void platinum_dat_sort_hashes(uint32_t* hashes, uint16_t* indices, const uint32_t size)
{
	uint32_t* nibbles = (uint32_t*)calloc(size, 4);
	
	/* Generate nibbles */
	for(uint32_t i = 0; i != size; ++i)
	{
		nibbles[i] = (hashes[i] & 0x70000000);
	}

	/* Bubble sort */
	for(uint32_t step = 0; step < size-1; ++step)
	{
		for(uint32_t i = 0; i < (size - step - 1); ++i)
		{
			if(nibbles[i] > nibbles[i + 1])
			{
				const uint32_t temp_hash = hashes[i];
				const uint16_t temp_idx = indices[i];
				const uint32_t temp_nibb = nibbles[i];
				
				hashes[i] = hashes[i+1];
				indices[i] = indices[i+1];
				nibbles[i] = nibbles[i+1];
				
				hashes[i+1] = temp_hash;
				indices[i+1] = temp_idx;
				nibbles[i+1] = temp_nibb;
			}
		}
	}
	
	free(nibbles);
}

void platinum_dat_gen_bucket_list(const uint32_t files_amount, uint16_t* bucket_offsets, const uint32_t* hashes, const uint32_t prehash_shift)
{
	for(uint32_t i = 0; i != files_amount; ++i)
	{
		const uint32_t bucket_off_id = hashes[i] >> prehash_shift;

		if(bucket_offsets[bucket_off_id] == 0xFFFF)
		{
			bucket_offsets[bucket_off_id] = i;
		}
	}
}

void platinum_dat_free_dat(DAT_FILE* dat)
{
	if(dat)
	{
		for(uint32_t i = 0; i != dat->header.files_amount; ++i)
		{
			free(dat->entries[i].name);
			free(dat->entries[i].data);
		}
	}
	
	free(dat->entries);
	
	free(dat->bucket_offsets);
	free(dat->hashes);
	free(dat->indices);
	
	free(dat);
}