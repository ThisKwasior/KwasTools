#pragma once

#include <stdint.h>

#include "mirage.h"

#define MAT_ANIM_METADATA_FIXED_SIZE		8
#define MAT_ANIM_METADATA_OFFSETS_OFFSET	(MIRAGE_INFO_SIZE+MAT_ANIM_METADATA_FIXED_SIZE)
#define MAT_ANIM_ENTRY_FIXED_SIZE			20

#define MAT_ANIM_TYPE_UNK0	0
#define MAT_ANIM_TYPE_UNK1	1
#define MAT_ANIM_TYPE_UNK2	2
#define MAT_ANIM_TYPE_UNK3	3
#define MAT_ANIM_TYPE_UNK4	4
#define MAT_ANIM_TYPE_UNK5	5
#define MAT_ANIM_TYPE_UNK6	6
#define MAT_ANIM_TYPE_UNK7	7
#define MAT_ANIM_TYPE_UNK8	8
#define MAT_ANIM_TYPE_UNK9	9
#define MAT_ANIM_TYPE_UNK10	10

typedef struct
{
	uint32_t material_name_offset;
	uint32_t anim_count;
	uint32_t* anim_offsets;
	
	/* Pointers in string_table */
	const char* material_name;
} MAT_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
	MIRAGE_KEYFRAME_SET* keyframe_sets;
	
	/* Pointer in string_table */
	const char* name;
} MAT_ANIM_ENTRY;

typedef struct
{
	MIRAGE_HEADER header;
	MIRAGE_INFO info;
	MAT_ANIM_METADATA metadata;
	MAT_ANIM_ENTRY* entries;
	MIRAGE_KEYFRAME* keyframes;
	char* string_table;
	MIRAGE_FOOTER footer;
} MAT_ANIM_FILE;

/*
	Functions
*/

MAT_ANIM_FILE* mat_anim_load_file(FU_FILE* file);

FU_FILE* mat_anim_save_to_fu_file(MAT_ANIM_FILE* file);

void mat_anim_load_metadata(FU_FILE* file, MAT_ANIM_FILE* mat);
void mat_anim_load_entries(FU_FILE* file, MAT_ANIM_FILE* mat);

void mat_anim_write_metadata(FU_FILE* file, MAT_ANIM_FILE* mat);
void mat_anim_write_entries(FU_FILE* file, MAT_ANIM_FILE* mat);

/* Print functions */
void mat_anim_print_metadata(MAT_ANIM_FILE* mat);
void mat_anim_print_entries(MAT_ANIM_FILE* mat);

void mat_anim_print_mat(MAT_ANIM_FILE* mat);