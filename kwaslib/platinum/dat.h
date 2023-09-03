#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/dir_list.h>

/* 
	Code here has been "borrowed" from NierDocs.
	Thank you very much!
	https://github.com/ArthurHeitmann/NierDocs/blob/master/tools/datRepacker/datRepacker.py
*/

#define DAT_BLOCK_SIZE 4096

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
DAT_FILE* dat_parse_dat(FU_FILE* file, const uint8_t fu_endian);
DAT_FILE* dat_parse_directory(const char* dir, const uint8_t do_align);

FU_FILE* dat_save_to_fu_file(DAT_FILE* dat, const uint8_t fu_endian);

void dat_gen_hash_data(DAT_FILE* dat);

uint32_t dat_hash_filename(const uint8_t* name);

uint32_t dat_bit_count(uint32_t value);
uint32_t dat_next_pow_of_2_bits(uint32_t value);
uint32_t dat_calc_prehash_shift(uint32_t value);

/* Bubble sort */
void dat_sort_hashes(uint32_t* hashes, uint16_t* indices, const uint32_t size);

void dat_gen_bucket_list(const uint32_t files_amount, uint16_t* bucket_offsets, const uint32_t* hashes, const uint32_t prehash_shift);

void dat_free_dat(DAT_FILE* dat);