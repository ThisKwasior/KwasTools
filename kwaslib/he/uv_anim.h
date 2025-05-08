#pragma once

/*
    uv-anim - allows for animating uv maps.
    
    Has two versions known - v2 and v3
    
    v2 is only able to animate one texture in material,
    defined in texture_name in metadata.
    It can only do translation.
    
    v3 can animate multiple textures in the same material.
    Texture name in the metadata is then `none` or `default`, 
    with targeted texture being the name of an animation.
    Supports translation, scale and rotation.
    
    v2 games:
        - Sonic Unleashed
        - Sonic Generations (2011)
        - Sonic Generations (2024)
        - Sonic Lost World
    
    v3 games:
        - Sonic Forces
        - Sonic Frontiers
        - Shadow Generations
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define UV_ANIM_HEADER_SIZE     (6*4)

#define UV_ANIM_TYPE_POS_X      0
#define UV_ANIM_TYPE_POS_Y      1
#define UV_ANIM_TYPE_ROTATION   2
#define UV_ANIM_TYPE_SCALE_X    3
#define UV_ANIM_TYPE_SCALE_Y    4

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} UV_ANIM_HEADER;

typedef struct
{
	uint32_t material_name_offset;
	uint32_t texture_name_offset;
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} UV_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
    
	CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
} UV_ANIM_ENTRY;

typedef struct
{
    UV_ANIM_HEADER header;
    UV_ANIM_METADATA metadata;
    CVEC entries; /* Array of UV_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} UV_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to UV_ANIM_FILE
*/
UV_ANIM_FILE* uv_anim_alloc();

/*
    Loads the uv-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
UV_ANIM_FILE* uv_anim_load_from_data(const uint8_t* data);

/*
    Exports an UV_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported uv anim data.
*/
FU_FILE* uv_anim_export_to_fu(UV_ANIM_FILE* uv);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void uv_anim_update(UV_ANIM_FILE* uv);

/*
    Frees all of the contents of the uv anim.
    
    Returns NULL.
*/
UV_ANIM_FILE* uv_anim_free(UV_ANIM_FILE* uv);

/*
    Returns a pointer to uv anim entry structure by id.
*/
UV_ANIM_ENTRY* uv_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC uv_anim_calc_offsets(UV_ANIM_FILE* uv);