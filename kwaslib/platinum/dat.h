#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
	uint8_t magic[4];				/* {'D','A','T',0} */
	uint32_t files_amount;			/* Amount of files in the container */
	uint32_t positions_offset;
	uint32_t extensions_offset;
	uint32_t names_offset;
	uint32_t sizes_offset;
	uint32_t data_offsets;
	uint32_t padding;				/* Zero */
} DAT_HEADER;

typedef struct
{
	uint32_t position;				/* Position in the file */
	uint8_t extension[4];			/* File extension */
	uint8_t* name;					/* File name */
	uint32_t size;					/* File size */
	uint8_t* data;					/* File data */
} DAT_FILE_ENTRY;

typedef struct
{
	DAT_HEADER header;				/* Fixed-length header */
	
	uint32_t entry_name_size;		/* Size for file names
									   First value in names section */
									   
	DAT_FILE_ENTRY* entries;		/* File entries */
} DAT_FILE;