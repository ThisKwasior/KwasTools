#pragma once

/*
    pt-anim - texture swapping on the fly.
    
    One version known - v2
    
    Has an additional section, that being names of textures.
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define PT_ANIM_HEADER_SIZE     (8*4)

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
	uint32_t texture_table_offset;
	uint32_t texture_table_size;
} PT_ANIM_HEADER;

typedef struct
{
	uint32_t material_name_offset;
	uint32_t texture_name_offset;
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} PT_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
    
	CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
	/* CVEC texture_length; */ /* Array of uint32_t */
	/* CVEC texture_start; */ /* Array of uint32_t */
} PT_ANIM_ENTRY;

typedef struct
{
    PT_ANIM_HEADER header;
    PT_ANIM_METADATA metadata;
    CVEC entries; /* Array of PT_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
    SU_STRING* texture_table;
} PT_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to PT_ANIM_FILE
*/
PT_ANIM_FILE* pt_anim_alloc();

/*
    Loads the pt-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
PT_ANIM_FILE* pt_anim_load_from_data(const uint8_t* data);

/*
    Exports an PT_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported pt anim data.
*/
FU_FILE* pt_anim_export_to_fu(PT_ANIM_FILE* pt);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void pt_anim_update(PT_ANIM_FILE* pt);

/*
    Frees all of the contents of the pt anim.
    
    Returns NULL.
*/
PT_ANIM_FILE* pt_anim_free(PT_ANIM_FILE* pt);

/*
    Returns a pointer to pt anim entry structure by id.
*/
PT_ANIM_ENTRY* pt_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC pt_anim_calc_offsets(PT_ANIM_FILE* pt);