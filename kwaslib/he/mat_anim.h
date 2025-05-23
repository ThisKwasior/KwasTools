#pragma once

/*
    mat-anim - Transforms the material on top of its transformations.
    
    One version known - v2
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define MAT_ANIM_HEADER_SIZE    (6*4)

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} MAT_ANIM_HEADER;

typedef struct
{
	uint32_t material_name_offset;
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} MAT_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
    
	CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
} MAT_ANIM_ENTRY;

typedef struct
{
    MAT_ANIM_HEADER header;
    MAT_ANIM_METADATA metadata;
    CVEC entries; /* Array of MAT_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} MAT_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to MAT_ANIM_FILE
*/
MAT_ANIM_FILE* mat_anim_alloc();

/*
    Loads the mat-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
MAT_ANIM_FILE* mat_anim_load_from_data(const uint8_t* data);

/*
    Exports an MAT_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported mat anim data.
*/
FU_FILE* mat_anim_export_to_fu(MAT_ANIM_FILE* mat);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void mat_anim_update(MAT_ANIM_FILE* mat);

/*
    Frees all of the contents of the mat anim.
    
    Returns NULL.
*/
MAT_ANIM_FILE* mat_anim_free(MAT_ANIM_FILE* mat);

/*
    Returns a pointer to mat anim entry structure by id.
*/
MAT_ANIM_ENTRY* mat_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC mat_anim_calc_offsets(MAT_ANIM_FILE* mat);