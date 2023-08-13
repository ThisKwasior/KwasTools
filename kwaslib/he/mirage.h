#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/path_utils.h>

#define MIRAGE_HEADER_SIZE			24
#define MIRAGE_INFO_SIZE			24
#define MIRAGE_KEYFRAME_SIZE		8
#define MIRAGE_INFO_OFFSET_CNT		6
#define MIRAGE_KEYFRAME_SET_SIZE	12

/*
	BIGGGG thanks to @ik-01 for the help with
	anim formats and working together on cam-anim format
*/

/*
	Files are in big endian on all platforms.
	Sections are padded to 4 bytes apparently.
*/

typedef struct
{
	uint32_t file_size;
	uint32_t root_node_type;
	uint32_t root_node_size;
	uint32_t root_node_offset;
	uint32_t footer_offset;
	uint32_t file_end_offset;
} MIRAGE_HEADER;

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} MIRAGE_INFO;

typedef struct
{
	float index; /* What were you cooking, Sonic Team */
	float value;
} MIRAGE_KEYFRAME;

typedef struct
{
	uint32_t offset_count;
	uint32_t* offsets;
} MIRAGE_FOOTER;

typedef struct
{
	uint8_t type;
	uint8_t flag2;
	uint8_t interpolation;
	uint8_t flag4;
	uint32_t length;
	uint32_t start;
} MIRAGE_KEYFRAME_SET;

/*
	Functions
*/
void mirage_read_header(FU_FILE* mirage, MIRAGE_HEADER* header);
void mirage_read_info(FU_FILE* mirage, MIRAGE_INFO* info);
MIRAGE_KEYFRAME* mirage_read_keyframes(FU_FILE* mirage, MIRAGE_INFO* info);
char* mirage_read_string_table(FU_FILE* mirage, MIRAGE_INFO* info);
void mirage_read_footer(FU_FILE* mirage, MIRAGE_HEADER* header, MIRAGE_FOOTER* footer);

void mirage_write_header(FU_FILE* mirage, MIRAGE_HEADER* header);
void mirage_write_info(FU_FILE* mirage, MIRAGE_INFO* info);
void mirage_write_keyframes(FU_FILE* mirage, MIRAGE_INFO* info, MIRAGE_KEYFRAME* keyframes);
void mirage_write_string_table(FU_FILE* mirage, MIRAGE_INFO* info, char* string_table);
void mirage_write_footer(FU_FILE* mirage, MIRAGE_HEADER* header, MIRAGE_FOOTER* footer);

/* Print functions */
void mirage_print_header(MIRAGE_HEADER* header);
void mirage_print_info(MIRAGE_INFO* info);
void mirage_print_keyframes(MIRAGE_INFO* info, MIRAGE_KEYFRAME* keyframes);
void mirage_print_string_table(MIRAGE_INFO* info, char* string_table);
void mirage_print_offsets(MIRAGE_FOOTER* footer);
