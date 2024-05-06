#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>

/*
	https://wiki.tockdom.com is an amazing resource!
	Thank you very much!
*/

#define BRRES_MAGIC_BRRES	(const char*)"bres"

/* Common structures */

typedef struct
{
	char magic[4];
	uint32_t length;
	uint32_t version;
	uint32_t outer_brres_offset;
	uint32_t* section_offsets;
	uint32_t name_offset;
	
	/* Helper fields, not in the format */
	char* name;
	uint32_t name_size;
} BRRES_SUB_HEADER;

typedef struct
{
	uint16_t entry_id;
	uint16_t flag; /* Always 0? */
	uint16_t left_idx;
	uint16_t right_idx;
	uint32_t name_offset;
	uint32_t data_offset;
	
	/* Helper fields, not in the format */
	char* name;
	uint32_t name_size;
} BRRES_ENTRY;

typedef struct
{
	uint32_t length;
	uint32_t entries_count;
	BRRES_ENTRY* entries; /* Size is 1 + entries_count */
} BRRES_GROUP;

typedef struct
{
	float index;
	float value;
	float tangent;
} BRRES_KEYFRAME;

typedef struct
{
	uint16_t frame_count;
	uint16_t unk_02;
	float frame_scale;
	BRRES_KEYFRAME* keyframes;
	
	/* Additional */
	float fixed_val;
	uint8_t fixed;
} BRRES_KEYFRAME_SET;

/* Functions */

void brres_read_group(FU_FILE* brres, BRRES_GROUP* group);

void brres_read_entry(FU_FILE* brres, BRRES_ENTRY* entry);

void brres_read_keyframe_set(FU_FILE* brres, BRRES_KEYFRAME_SET* kfs);
void brres_read_keyframe(FU_FILE* brres, BRRES_KEYFRAME* keyframe);

void brres_free_sub_header(BRRES_SUB_HEADER* sub_header);
void brres_free_group(BRRES_GROUP* group);

void brres_print_group(BRRES_GROUP* group);

char* brres_read_str_and_back(FU_FILE* file, const uint32_t offset, uint32_t* name_size);