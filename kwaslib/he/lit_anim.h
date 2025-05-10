#pragma once

/*
    lit-anim - animates light points
    
    One version known - v2
    
    Implementation based on Shadow Generations lit-anim
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define LIT_ANIM_HEADER_SIZE        (6*4)

#define LIT_ANIM_TYPE_COLOR_RED     4
#define LIT_ANIM_TYPE_COLOR_GREEN   5
#define LIT_ANIM_TYPE_COLOR_BLUE    6

typedef struct
{
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} LIT_ANIM_HEADER;

typedef struct
{
	uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} LIT_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
    uint8_t light_type;
    uint8_t flag2;
    uint8_t flag3;
    uint8_t flag4;
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;

    float unk_00;
    float unk_01;
    float unk_02;
    float unk_03;
    
    float color_red;        // 04
    float color_green;      // 05
    float color_blue;       // 06
    float unk_07;
    
    float unk_08;
    float unk_09;
    float unk_0A;
    float unk_0B;
    
    float unk_0C;
    float unk_0D;
    float unk_0E;
    float intensity;        // 0F
    
    float unk_10;
    float unk_11;
    float unk_12;
    float unk_13;
    
    float unk_14;
    float unk_15;
    float unk_16;
    float unk_17;
    
    float unk_18;
    float unk_19;
    float unk_1A;
    float unk_1B;
    
	CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
} LIT_ANIM_ENTRY;

typedef struct
{
    LIT_ANIM_HEADER header;
    LIT_ANIM_METADATA metadata;
    CVEC entries; /* Array of LIT_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} LIT_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to LIT_ANIM_FILE
*/
LIT_ANIM_FILE* lit_anim_alloc();

/*
    Loads the lit-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
LIT_ANIM_FILE* lit_anim_load_from_data(const uint8_t* data);

/*
    Exports an LIT_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported lit anim data.
*/
FU_FILE* lit_anim_export_to_fu(LIT_ANIM_FILE* lit);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void lit_anim_update(LIT_ANIM_FILE* lit);

/*
    Frees all of the contents of the lit anim.
    
    Returns NULL.
*/
LIT_ANIM_FILE* lit_anim_free(LIT_ANIM_FILE* lit);

/*
    Returns a pointer to lit anim entry structure by id.
*/
LIT_ANIM_ENTRY* lit_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC lit_anim_calc_offsets(LIT_ANIM_FILE* lit);