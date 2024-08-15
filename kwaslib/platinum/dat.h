#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/dir_list.h>
#include <kwaslib/core/data/dbl_link_list.h>

/* 
	Code here has been "borrowed" from NierDocs and xxk-i.
	Thank you very much!
	https://github.com/ArthurHeitmann/NierDocs/blob/master/tools/datRepacker/datRepacker.py
	https://github.com/xxk-i/DATrepacker
*/

#define DAT_MAGIC				(const char*)"DAT\0"
#define DAT_DEFAULT_BLOCK_SIZE	2048

typedef struct
{
	uint8_t magic[4];				/* {'D','A','T',0} */
	uint32_t file_count;			/* Amount of files in the container */
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
	uint8_t* name;					/* File name, NULL terminated */
	uint8_t* data;					/* File data */
} DAT_FILE_ENTRY;

typedef struct
{
	uint32_t hash;
	uint32_t pos;
	uint32_t index;
} DAT_HASH;

typedef struct
{
	uint32_t prehash_shift;
	uint32_t bucket_offset; /* Constant 0x10 */
	uint32_t hashes_offset;
	uint32_t indices_offset;
	
	uint16_t* bucket;
	uint32_t bucket_size;
	
	uint32_t* order;
	
	DAT_HASH* hashes;
} DAT_HASHES;

typedef struct
{
	DAT_HEADER header;				/* Fixed-length header */
	
	uint32_t entry_name_size;		/* Size for file names
									   First value in names section */
									   
	DBL_LIST_NODE* entries;			/* File entries in doubly linked list */
	
	DAT_HASHES hashes;				/* Section pointed to by header.hashes_offset */
} DAT_FILE;

/*
	Functions
*/

DAT_FILE* dat_parse(FU_FILE* file, const uint8_t fu_endian);
FU_FILE* dat_save_to_fu_file(DAT_FILE* dat, const uint8_t fu_endian);

/* 
	Updates fields in the structure -
	header, entry_name_size and hashes based on entries list.
	
	Everything is calculated based on entries
	to make things simpler for data manipulation in code.
	
	The only thing not needed in each entry is the position;
	it's gonna get updated too since we don't know the position
	beforehand due to alignment.
	You can set it to 0 when calling `dat_entry_from_data()`.
*/
void dat_update(DAT_FILE* dat, const uint32_t alignment);

DAT_FILE_ENTRY* dat_entry_from_data(const uint32_t position,
									const uint8_t* extension,
									const uint32_t name_size,
									const uint8_t* name,
									const uint32_t size,
									const uint8_t* data);

DAT_HEADER dat_generate_header(DBL_LIST_NODE* entries,
							   DAT_HASHES* hashes,
							   const uint32_t entry_name_size);
							   
DAT_HASHES dat_generate_hashes(DBL_LIST_NODE* entries);

void dat_generate_positions(DAT_HEADER* header, DAT_HASHES* hashes,
							DBL_LIST_NODE* entries, const uint32_t alignment);

DBL_LIST_NODE* dat_append_entry(DBL_LIST_NODE* entries, DAT_FILE_ENTRY* entry);
DAT_FILE_ENTRY* dat_get_entry_node(DBL_LIST_NODE* entries, const uint64_t pos);

/* Doesn't free the struct itself */							
void dat_free_entry(DAT_FILE_ENTRY* entry);
void dat_free_hashes(DAT_HASHES* hashes);

/* Doesn't free the DAT itself */	
void dat_free(DAT_FILE* dat);

/* Helper */
const uint8_t dat_bit_length(const uint64_t value);
const uint8_t dat_calc_prehash_shift(const uint32_t file_count);
const uint8_t dat_calc_bucket_size(const uint8_t prehash_shift);
const uint32_t dat_get_max_name_size(DBL_LIST_NODE* entries);
const uint32_t dat_calc_hash_from_name(const uint8_t* name, const uint64_t length);