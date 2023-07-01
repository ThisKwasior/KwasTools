#pragma once

#include <stdint.h>

#include <kwaslib/utils/io/file_utils.h>
#include <kwaslib/utils/io/dir_list.h>

/* 
	Code here has been "borrowed" from NierDocs.
	Thank you very much!
	https://github.com/ArthurHeitmann/NierDocs/blob/master/tools/datRepacker/datRepacker.py
*/

typedef struct
{
	uint8_t magic[4];				/* {'D','A','T',0} */
	uint32_t files_amount;			/* Amount of files in the container */
	uint32_t positions_offset;
	uint32_t extensions_offset;
	uint32_t names_offset;
	uint32_t sizes_offset;
	uint32_t hashes_offset;
	uint32_t unk1C;					/* Zero */
} DAT_HEADER;

typedef struct
{
	uint32_t position;				/* Position in the file */
	uint32_t size;					/* File size */
	uint8_t extension[4];			/* File extension */
	uint8_t* name;					/* File name */
	uint8_t* data;					/* File data */
} DAT_FILE_ENTRY;

typedef struct
{
	DAT_HEADER header;				/* Fixed-length header */
	
	uint32_t entry_name_size;		/* Size for file names
									   First value in names section */
									   
	DAT_FILE_ENTRY* entries;		/* File entries */
	
	uint32_t prehash_shift;
	uint32_t bucket_offsets_offset; /* Constant 0x10 */
	uint32_t hashes_offset;
	uint32_t indices_offset;
	
	uint32_t bucket_offsets_size;
	
	uint16_t* bucket_offsets;
	uint32_t* hashes; /* Size is header.files_amount / CRC32 encode of a file name. */
	uint16_t* indices; /* Size is header.files_amount */
} DAT_FILE;

/*
	Functions
*/
DAT_FILE* platinum_dat_parse_dat(FU_FILE* file, const uint8_t fu_endian);
DAT_FILE* platinum_dat_parse_directory(const char* dir);

FU_FILE* platinum_dat_save_to_fu_file(DAT_FILE* dat, const uint8_t fu_endian);

void platinum_dat_gen_hash_data(DAT_FILE* dat);

uint32_t platinum_dat_hash_filename(const uint8_t* name);

uint32_t platinum_dat_bit_count(uint32_t value);
uint32_t platinum_dat_next_pow_of_2_bits(uint32_t value);
uint32_t platinum_dat_calc_prehash_shift(uint32_t value);

/* Bubble sort */
void platinum_dat_sort_hashes(uint32_t* hashes, uint16_t* indices, const uint32_t size);

void platinum_dat_gen_bucket_list(const uint32_t files_amount, uint16_t* bucket_offsets, const uint32_t* hashes, const uint32_t prehash_shift);

void platinum_dat_free_dat(DAT_FILE* dat);