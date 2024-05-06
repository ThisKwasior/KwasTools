#pragma once

#include <stdint.h>

#include "mirage.h"

#define UV_ANIM_METADATA_FIXED_SIZE			12
#define UV_ANIM_METADATA_OFFSETS_OFFSET		(MIRAGE_INFO_SIZE+UV_ANIM_METADATA_FIXED_SIZE)
#define UV_ANIM_ENTRY_FIXED_SIZE			20

#define UV_ANIM_TYPE_POSX	0
#define UV_ANIM_TYPE_POSY	1
#define UV_ANIM_TYPE_ROT	2
#define UV_ANIM_TYPE_SCALEX	3
#define UV_ANIM_TYPE_SCALEY 4

typedef struct
{
	uint32_t material_name_offset;
	uint32_t texture_name_offset; /* Probably */
	uint32_t anim_count;
	uint32_t* anim_offsets;
	
	/* Pointers in string_table */
	const char* material_name;
	const char* texture_name;
} UV_ANIM_METADATA;

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
} UV_ANIM_ENTRY;

typedef struct
{
	MIRAGE_HEADER header;
	MIRAGE_INFO info;
	UV_ANIM_METADATA metadata;
	UV_ANIM_ENTRY* entries;
	MIRAGE_KEYFRAME* keyframes;
	char* string_table;
	MIRAGE_FOOTER footer;
} UV_ANIM_FILE;

/*
	Functions
*/

UV_ANIM_FILE* uv_anim_load_file(FU_FILE* file);

FU_FILE* uv_anim_save_to_fu_file(UV_ANIM_FILE* file);

void uv_anim_load_metadata(FU_FILE* file, UV_ANIM_FILE* uv);
void uv_anim_load_entries(FU_FILE* file, UV_ANIM_FILE* uv);

void uv_anim_write_metadata(FU_FILE* file, UV_ANIM_FILE* uv);
void uv_anim_write_entries(FU_FILE* file, UV_ANIM_FILE* uv);

/* Print functions */
void uv_anim_print_metadata(UV_ANIM_FILE* uv);
void uv_anim_print_entries(UV_ANIM_FILE* uv);

void uv_anim_print_uv(UV_ANIM_FILE* uv);