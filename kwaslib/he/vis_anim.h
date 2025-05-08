#pragma once

/*
    vis-anim - controls visibility of mesh.
    
    Only seen one version - v1
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define VIS_ANIM_HEADER_SIZE     (6*4)

#define VIS_ANIM_TYPE_VISIBILITY    0

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} VIS_ANIM_HEADER;

typedef struct
{
	uint32_t model_name_offset;
	uint32_t unk_name_offset;
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} VIS_ANIM_METADATA;

typedef struct
{
    uint32_t name_offset;
    float frame_rate;
    float start_frame;
    float end_frame;
	uint32_t keyframe_set_count;
    
	CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
} VIS_ANIM_ENTRY;

typedef struct
{
    VIS_ANIM_HEADER header;
    VIS_ANIM_METADATA metadata;
    CVEC entries; /* Array of VIS_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} VIS_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to VIS_ANIM_FILE
*/
VIS_ANIM_FILE* vis_anim_alloc();

/*
    Loads the vis-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
VIS_ANIM_FILE* vis_anim_load_from_data(const uint8_t* data);

/*
    Exports an VIS_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported vis anim data.
*/
FU_FILE* vis_anim_export_to_fu(VIS_ANIM_FILE* vis);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void vis_anim_update(VIS_ANIM_FILE* vis);

/*
    Frees all of the contents of the vis anim.
    
    Returns NULL.
*/
VIS_ANIM_FILE* vis_anim_free(VIS_ANIM_FILE* vis);

/*
    Returns a pointer to vis anim entry structure by id.
*/
VIS_ANIM_ENTRY* vis_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC vis_anim_calc_offsets(VIS_ANIM_FILE* vis);