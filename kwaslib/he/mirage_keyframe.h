#pragma once

#include <kwaslib/core/data/cvector.h>

#define MIRAGE_KEYFRAME_SIZE        0x8
#define MIRAGE_KEYFRAME_SET_SIZE    0xC

#define MIRAGE_KFS_INTERP_CONSTANT  1
#define MIRAGE_KFS_INTERP_LINEAR    0

typedef struct
{
	uint8_t type; /* Used in every anim besides morph-anim according to Skyth */
	uint8_t flag2; /* Used in cam and lit anims according to Skyth. Purpose unknown */
	uint8_t interpolation; /* Used in uv-anim only. 1 for constant, 0 for linear */
	uint8_t flag4; /* Unused */
	uint32_t length;
	uint32_t start;
} MIRAGE_KEYFRAME_SET;

typedef struct
{
	float index; /* What were you cooking, Sonic Team */
	float value;
} MIRAGE_KEYFRAME;

/*
    Implementation
*/

/*
    Reads keyframe set from provided pointer.
*/
const MIRAGE_KEYFRAME_SET mirage_read_kfs_from_data(const uint8_t* data);

/*
    Reads all keyframe sets from data to a created (empty) cvector.
*/
void mirage_read_keyframe_sets_from_data(const uint8_t* data, const uint32_t kfs_count, CVEC keyframe_sets);

/*
    Reads keyframes from keyframe section of an animation
    to a created (empty) cvector.
*/
void mirage_read_keyframes_from_data(const uint8_t* data, const uint32_t data_size, CVEC keyframes);

/*
    Creates and pushed a keyframe to the cvector.
*/
void mirage_push_keyframe(CVEC keyframes, const float index, const float value);

/*
    Creates and pushed a keyframe set to the cvector.
*/
void mirage_push_kfs(CVEC keyframe_sets, const uint8_t type, 
                     const uint8_t flag2, const uint8_t interpolation,
                     const uint8_t flag4, const uint32_t length,
                     const uint32_t start);

/*
    Gets a keyframe structure pointer from a cvector.
*/
MIRAGE_KEYFRAME* mirage_get_kf_by_id(CVEC keyframes, const uint32_t id);

/*
    Gets a keyframe_set structure pointer from a cvector.
*/
MIRAGE_KEYFRAME_SET* mirage_get_kfs_by_id(CVEC keyframe_sets, const uint32_t id);