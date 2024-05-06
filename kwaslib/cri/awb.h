#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/path_utils.h>

/*
    CRIWARE archive for audio, sometimes accompanies ACB file with metadata.
	
	Endianness on PC is little.
*/

/*
	At the end of the offset table is an extra one
	that's just the end of the file.
*/

#define AWB_MAGIC		"AFS2"
#define AWB_ID_OFFSET	0x10

typedef struct
{
	char magic[4]; /* AFS2 */
	uint8_t version;
	uint8_t offset_size; /* 2 or 4 */
	uint16_t id_align; /* Usually 2, sometimes 4 */
	uint32_t file_count;
	uint16_t alignment;
	uint16_t subkey; /* decryption key */
} AWB_HEADER;

typedef struct
{
	uint16_t id;
	uint32_t offset;
	uint8_t* data;
	uint32_t size;
} AWB_ENTRY;

typedef struct
{
	AWB_HEADER header;
	AWB_ENTRY* entries;
	
	uint32_t file_offset;
	uint8_t no_data;
} AWB_FILE;

/*
	Functions
*/

AWB_FILE* awb_read_file(FU_FILE* awb, const uint32_t offset);
AWB_FILE* awb_parse_directory(const char* dir);

void awb_extract_to_folder(AWB_FILE* afs2, PU_STRING* dir);

void awb_read_header(FU_FILE* awb, AWB_FILE* afs2);
void awb_read_entries(FU_FILE* awb, AWB_FILE* afs2);

FU_FILE* awb_write_file(AWB_FILE* afs2);
void awb_write_header(FU_FILE* awb, AWB_FILE* afs2);
void awb_write_entries(FU_FILE* awb, AWB_FILE* afs2);

void awb_default_header(AWB_HEADER* header, const uint32_t file_count);
AWB_FILE* awb_alloc_file();
AWB_ENTRY* awb_alloc_entries(const uint32_t entry_count);

/* 
	Check if it's just a header (StreamAwbHeader in ACB).
	Sets the no_data to 1 if there's no file data.
	Requires the entries[file_count] to contain the file size.
	TODO: Don't do it like that
*/
uint8_t awb_check_for_data(FU_FILE* awb, AWB_FILE* afs2);

void awb_print(AWB_FILE* afs2);
void awb_free(AWB_FILE* afs2);