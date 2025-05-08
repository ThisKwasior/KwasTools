#pragma once

/*
    morph-anim - animates "morphs" of the mesh (also known as shape keys).
    
    One version known - v2
    
    Used in:
        - Sonic Unleashed (2008)
        - Sonic Generations (2011)
        - Sonic Generations (2024)
        
    Keyframe sets are different from the other anim files,
    there's an additional offset at the start with the name of the morph.
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define MORPH_ANIM_HEADER_SIZE          (6*4)

#define MORPH_ANIM_TYPE_WEIGHT_UNL      0
#define MORPH_ANIM_TYPE_WEIGHT_GENS     1

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} MORPH_ANIM_HEADER;

typedef struct
{
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} MORPH_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
    
	CVEC morph_names_offsets;   /* Array of uint32_t */
	CVEC keyframe_sets;         /* Array of MIRAGE_KEYFRAME_SET */
} MORPH_ANIM_ENTRY;

typedef struct
{
    MORPH_ANIM_HEADER header;
    MORPH_ANIM_METADATA metadata;
    CVEC entries; /* Array of MORPH_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} MORPH_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to MORPH_ANIM_FILE
*/
MORPH_ANIM_FILE* morph_anim_alloc();

/*
    Loads the morph-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
MORPH_ANIM_FILE* morph_anim_load_from_data(const uint8_t* data);

/*
    Exports an MORPH_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported morph anim data.
*/
FU_FILE* morph_anim_export_to_fu(MORPH_ANIM_FILE* morph);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void morph_anim_update(MORPH_ANIM_FILE* morph);

/*
    Frees all of the contents of the morph anim.
    
    Returns NULL.
*/
MORPH_ANIM_FILE* morph_anim_free(MORPH_ANIM_FILE* morph);

/*
    Returns a pointer to morph anim entry structure by id.
*/
MORPH_ANIM_ENTRY* morph_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC morph_anim_calc_offsets(MORPH_ANIM_FILE* morph);