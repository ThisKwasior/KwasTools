#pragma once

#include <stdint.h>

#include <kwaslib/core/math/vec.h>

#include "mirage.h"

#define CAM_ANIM_METADATA_FIXED_SIZE		4
#define CAM_ANIM_METADATA_OFFSETS_OFFSET	(MIRAGE_INFO_SIZE+CAM_ANIM_METADATA_FIXED_SIZE)
#define CAM_ANIM_ENTRY_FIXED_SIZE			80

#define CAM_ANIM_TYPE_CAM_X		0
#define CAM_ANIM_TYPE_CAM_Z		1
#define CAM_ANIM_TYPE_CAM_Y		2
#define CAM_ANIM_TYPE_CAM_ROT_X	3
#define CAM_ANIM_TYPE_CAM_ROT_Z	4
#define CAM_ANIM_TYPE_CAM_ROT_Y	5
#define CAM_ANIM_TYPE_AIM_X		6
#define CAM_ANIM_TYPE_AIM_Z		7
#define CAM_ANIM_TYPE_AIM_Y		8
#define CAM_ANIM_TYPE_TWIST		9
#define CAM_ANIM_TYPE_Z_NEAR	10
#define CAM_ANIM_TYPE_Z_FAR		11
#define CAM_ANIM_TYPE_FOV		12
#define CAM_ANIM_TYPE_ASPECT	13

typedef struct
{
	uint32_t anim_count;
	uint32_t* anim_offsets;
} CAM_ANIM_METADATA;

typedef struct
{
	uint32_t name_offset;
	uint8_t rot_or_aim; /* 0 for rotation fields, 1 for aim+twist */
	uint8_t flag2; /* 0 */
	uint8_t flag3; /* 0 */
	uint8_t flag4; /* 0 */
	float frame_rate;
	float start_frame;
	float end_frame;
	uint32_t keyframe_set_count;
	VEC3_FLOAT cam_position;
	VEC3_FLOAT cam_rotation; /* Redundant with rot_or_aim=1 */
	VEC3_FLOAT aim_position;
	float twist; /* aim z rotation */
	float z_near;
	float z_far;
	float fov;
	float aspect_ratio;

	MIRAGE_KEYFRAME_SET* keyframe_sets;
	
	/* Pointer in string_table */
	const char* name;
} CAM_ANIM_ENTRY;

typedef struct
{
	MIRAGE_HEADER header;
	MIRAGE_INFO info;
	CAM_ANIM_METADATA metadata;
	CAM_ANIM_ENTRY* entries;
	MIRAGE_KEYFRAME* keyframes;
	char* string_table;
	MIRAGE_FOOTER footer;
} CAM_ANIM_FILE;

/*
	Functions
*/

CAM_ANIM_FILE* cam_anim_load_file(FU_FILE* file);

FU_FILE* cam_anim_save_to_fu_file(CAM_ANIM_FILE* file);

void cam_anim_load_metadata(FU_FILE* file, CAM_ANIM_FILE* cam);
void cam_anim_load_entries(FU_FILE* file, CAM_ANIM_FILE* cam);

void cam_anim_write_metadata(FU_FILE* file, CAM_ANIM_FILE* cam);
void cam_anim_write_entries(FU_FILE* file, CAM_ANIM_FILE* cam);

/* Print functions */
void cam_anim_print_metadata(CAM_ANIM_FILE* cam);
void cam_anim_print_entries(CAM_ANIM_FILE* cam);

void cam_anim_print_cam(CAM_ANIM_FILE* cam);