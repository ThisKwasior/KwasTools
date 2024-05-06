#pragma once

#include "brres.h"

#define	SCN0_MAGIC			(const char*)"SCN0"
#define SCN0_V4				4
#define SCN0_V5				5
#define SCN0_V4_SECTIONS	6
#define SCN0_V5_SECTIONS	7

#define SCN0_CAMERA_KFSETS	15

/* Structures */

typedef enum
{
	SCN0_SECTION_SPEC_LIGHT	= 0,
	SCN0_SECTION_LIGHT_SET	= 1,
	SCN0_SECTION_AMBIENT	= 2,
	SCN0_SECTION_LIGHT		= 3,
	SCN0_SECTION_FOG		= 4,
	SCN0_SECTION_CAMERA		= 5,
	SCN0_SECTION_7			= 6		/* Only in version 5 */
} SCN0_SECTION_OFFSETS;

typedef enum
{
	SCN0_CAMERA_POS_X			= 0,
	SCN0_CAMERA_POS_Y			= 1,
	SCN0_CAMERA_POS_Z			= 2,
	SCN0_CAMERA_ASPECT			= 3,
	SCN0_CAMERA_NEAR_Z			= 4,
	SCN0_CAMERA_FAR_Z			= 5,
	SCN0_CAMERA_ROT_X			= 6,
	SCN0_CAMERA_ROT_Y			= 7,
	SCN0_CAMERA_ROT_Z			= 8,
	SCN0_CAMERA_AIM_X			= 9,
	SCN0_CAMERA_AIM_Y			= 10,
	SCN0_CAMERA_AIM_Z			= 11,
	SCN0_CAMERA_TWIST			= 12,
	SCN0_CAMERA_FOV_Y			= 13,
	SCN0_CAMERA_ORTHO_HEIGHT	= 14
} SCN0_CAMERA_FLAGS1_ORDERED;

typedef enum
{
	SCN0_CAMERA_ROTATE	= 0,
	SCN0_CAMERA_AIM		= 1
} SCN0_CAMERA_TYPES;

typedef struct
{
	uint32_t unk_0; /* 0??? */
	uint16_t frame_count;
	uint16_t spec_light_count;
	uint32_t looping;
	uint16_t light_sets;
	uint16_t ambient_count;
	uint16_t light_count;
	uint16_t fog_count;
	uint16_t cam_count;
	uint16_t unk_count;
} SCN0_HEADER;

typedef struct
{
	uint32_t length;
	int32_t scn0_offset; /* Can be negative */
	uint32_t name_offset; 
	uint32_t node_index;
	uint32_t real_index;
	
	/* Not in the structure */
	char* name;
	uint32_t name_size; 
} SCN0_SECTION_HEADER;

typedef union
{
	struct
	{	
		uint8_t ortho_height_const : 1;
		uint8_t aim_x_const : 1;
		uint8_t aim_y_const : 1;
		uint8_t aim_z_const : 1;
		uint8_t twist_const : 1;
		uint8_t rot_x_const : 1;
		uint8_t rot_y_const : 1;
		uint8_t rot_z_const : 1;
		
		uint8_t unk : 1;
		uint8_t pos_x_const : 1;
		uint8_t pos_y_const : 1;
		uint8_t pos_z_const : 1;
		uint8_t aspect_const : 1;
		uint8_t near_const : 1;
		uint8_t far_const : 1;
		uint8_t fov_y_const : 1;
	};
	
	uint16_t data;
} SCN0_CAMERA_FLAGS1;

typedef union
{
	struct
	{	
		uint8_t unk_0x00;
		
		uint8_t none : 1;
		uint8_t camera_type : 1;
		uint8_t always_on : 1;
		uint8_t unk_5 : 5;
	};
	
	uint16_t data;
} SCN0_CAMERA_FLAGS2;

typedef struct
{
	uint32_t projection; /* 0 for perspective, 1 for ortographic */
	
	SCN0_CAMERA_FLAGS1 flags1;
	SCN0_CAMERA_FLAGS2 flags2;

	int32_t user_data_offset;
	
	float pos_x;
	float pos_y;
	float pos_z;
	float aspect;
	float near_z;
	float far_z;
	float rot_x;
	float rot_y;
	float rot_z;
	float aim_x;
	float aim_y;
	float aim_z;
	float twist;
	float fov_y;
	float ortho_height;
} SCN0_CAMERA;

typedef struct
{
	SCN0_SECTION_HEADER header;
	
	/* TODO: Implement everything else besides camera */
	union
	{
		SCN0_CAMERA cam;
	};
	
	BRRES_KEYFRAME_SET* kf_sets;

} SCN0_SECTION;

typedef struct
{
	BRRES_SUB_HEADER sub_header;
	SCN0_HEADER header;
	
	/* Yummy stack */
	SCN0_SECTION* sections[SCN0_V5_SECTIONS];
} SCN0_FILE;

/* Functions */

SCN0_FILE* scn0_read_file(FU_FILE* scn0);

void scn0_read_sub_header(FU_FILE* scn0, BRRES_SUB_HEADER* sub_header);

void scn0_read_header(FU_FILE* scn0, SCN0_HEADER* header);

void scn0_read_sections(FU_FILE* scn0, SCN0_FILE* scn0f);
void scn0_read_section_header(FU_FILE* scn0, SCN0_SECTION_HEADER* header);

void scn0_read_camera(FU_FILE* scn0, SCN0_CAMERA* camera);
void scn0_read_camera_kf_sets(FU_FILE* scn0, SCN0_SECTION* camera);

SCN0_FILE* scn0_alloc_file();

void scn0_print(SCN0_FILE* scn0);

void scn0_free(SCN0_FILE* scn0);