#pragma once

#include <stdint.h>

#include "mirage.h"

#define VIS_ANIM_METADATA_FIXED_SIZE		12
#define VIS_ANIM_METADATA_OFFSETS_OFFSET	(MIRAGE_INFO_SIZE+VIS_ANIM_METADATA_FIXED_SIZE)
#define VIS_ANIM_ENTRY_FIXED_SIZE			20

#define VIS_ANIM_TYPE_UNK0	0

typedef struct
{
	uint32_t material_name_offset;
	uint32_t texture_name_offset;
	uint32_t anim_count;
	uint32_t* anim_offsets;
	
	/* Pointers in string_table */
	const char* material_name;
	const char* texture_name;
} VIS_ANIM_METADATA;

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
} VIS_ANIM_ENTRY;

typedef struct
{
	MIRAGE_HEADER header;
	MIRAGE_INFO info;
	VIS_ANIM_METADATA metadata;
	VIS_ANIM_ENTRY* entries;
	MIRAGE_KEYFRAME* keyframes;
	char* string_table;
	MIRAGE_FOOTER footer;
} VIS_ANIM_FILE;

/*
	Functions
*/

VIS_ANIM_FILE* vis_anim_load_file(FU_FILE* file);

FU_FILE* vis_anim_save_to_fu_file(VIS_ANIM_FILE* file);

void vis_anim_load_metadata(FU_FILE* file, VIS_ANIM_FILE* vis);
void vis_anim_load_entries(FU_FILE* file, VIS_ANIM_FILE* vis);

void vis_anim_write_metadata(FU_FILE* file, VIS_ANIM_FILE* vis);
void vis_anim_write_entries(FU_FILE* file, VIS_ANIM_FILE* vis);

/* Print functions */
void vis_anim_print_metadata(VIS_ANIM_FILE* vis);
void vis_anim_print_entries(VIS_ANIM_FILE* vis);

void vis_anim_print_mat(VIS_ANIM_FILE* vis);