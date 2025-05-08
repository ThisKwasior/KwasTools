#pragma once

/*
    cam-anim - animates the ingame camera.
    
    One version known - v2
    
    Cameras can either use:
        - euler angles for rotation (0)
        - a point in space as aim (1)
    specified in the animation header.
*/

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>

#define CAM_ANIM_HEADER_SIZE     (6*4)

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
	uint32_t metadata_offset;
	uint32_t metadata_size;
	uint32_t keyframes_offset;
	uint32_t keyframes_size;
	uint32_t string_table_offset;
	uint32_t string_table_size;
} CAM_ANIM_HEADER;

typedef struct
{
    uint32_t anim_count;
	CVEC anim_offsets;  /* array of uint32_t */
} CAM_ANIM_METADATA;

typedef struct
{
    uint32_t name_offset;
	uint8_t rot_or_aim; /* 0 for rotation, 1 for aim+twist */
	uint8_t flag2; /* 0 */
	uint8_t flag3; /* 0 */
	uint8_t flag4; /* 0 */
	float frame_rate;
	float start_frame;
	float end_frame;
    uint32_t keyframe_set_count;
    float cam_pos_x;
    float cam_pos_z;
    float cam_pos_y;
    float cam_rot_x;
    float cam_rot_z;
    float cam_rot_y;
    float aim_pos_x;
    float aim_pos_z;
    float aim_pos_y;
    float twist;
    float z_near;
    float z_far;
    float fov;
    float aspect_ratio;
    
    CVEC keyframe_sets; /* Array of MIRAGE_KEYFRAME_SET */
} CAM_ANIM_ENTRY;

typedef struct
{
    CAM_ANIM_HEADER header;
    CAM_ANIM_METADATA metadata;
    CVEC entries; /* Array of CAM_ANIM_ENTRY */
    CVEC keyframes; /* Array of MIRAGE_KEYFRAME */
    SU_STRING* string_table;
} CAM_ANIM_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to CAM_ANIM_FILE
*/
CAM_ANIM_FILE* cam_anim_alloc();

/*
    Loads the cam-anim data from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
CAM_ANIM_FILE* cam_anim_load_from_data(const uint8_t* data);

/*
    Exports an CAM_ANIM_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported cam anim data.
*/
FU_FILE* cam_anim_export_to_fu(CAM_ANIM_FILE* cam);

/*
    Updates structures for export.
    
    Overwrites all fields in the header.
    Overwrites anim_count and recalculates anim_offsets according to entries vector.
    Aligns string_table to 4 bytes for export.
*/
void cam_anim_update(CAM_ANIM_FILE* cam);

/*
    Frees all of the contents of the cam anim.
    
    Returns NULL.
*/
CAM_ANIM_FILE* cam_anim_free(CAM_ANIM_FILE* cam);

/*
    Returns a pointer to cam anim entry structure by id.
*/
CAM_ANIM_ENTRY* cam_anim_get_entry_by_id(CVEC entries, const uint32_t id);

/*
    Returns a vector with offsets for offset table in mirage file.
*/
CVEC cam_anim_calc_offsets(CAM_ANIM_FILE* cam);