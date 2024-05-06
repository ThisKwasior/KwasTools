#pragma once

#include "brres.h"

#define	SRT0_MAGIC			(const char*)"SRT0"
#define SRT0_V4				4
#define SRT0_V5				5
#define SRT0_V4_SECTIONS	1
#define SRT0_V5_SECTIONS	2

/* Structures */

typedef enum
{
	SRT0_SECTION_ANIM_DATA	= 0,
	SRT0_SECTION_UNKNOWN	= 1		/* Only in version 5 */
} SRT0_SECTION_OFFSETS;

typedef enum
{
	SRT0_MODE_MAYA		= 0x0000,
	SRT0_MODE_XSI		= 0x0001,
	SRT0_MODE_3DSMAX	= 0x0002,
} SRT0_MATRIX_MODES;

typedef struct
{
	uint32_t unk_0; /* 0??? */
	uint16_t frame_count;
	uint16_t anim_data_count;
	uint32_t matrix_mode;
	uint32_t looping; /* 0 - disabled, 1 - enabled */
} SRT0_HEADER;
 
typedef union
{
	struct
	{
		uint16_t unk_1f;

		uint8_t	fixed_x_translation : 1;
		uint8_t	fixed_y_translation : 1;
		uint8_t	unk_a : 1;
		uint8_t	unk_b : 1;
		uint8_t	unk_c : 1;
		uint8_t	unk_d : 1;
		uint8_t	unk_e : 1;
		uint8_t	unk_f : 1;

		uint8_t	always_set_unk : 1;
		uint8_t	scale_one : 1;
		uint8_t	rot_zero : 1;
		uint8_t	translate_zero : 1;
		uint8_t	isotropic_scale : 1;
		uint8_t	fixed_x_scale : 1;
		uint8_t	fixed_y_scale : 1;
		uint8_t	fixed_rotation : 1;
	};
	
	uint32_t data;
} SRT0_TEX_ANIM_TYPE_CODE;

typedef struct
{
	float index;
	float value;
	float tangent;
} SRT0_TEX_ANIM_FRAME_INFO;

typedef struct
{
	uint16_t frame_count;
	uint16_t unk_02;
	float frame_scale;
	SRT0_TEX_ANIM_FRAME_INFO* keyframes;
	
	/* Additional */
	float fixed_val;
	uint8_t fixed;
} SRT0_TEX_ANIM_FRAME_DATA;

typedef struct
{
	SRT0_TEX_ANIM_TYPE_CODE code;
	
	/* Scale X, Scale Y, Rotation, Trans X, Trans Y */
	SRT0_TEX_ANIM_FRAME_DATA frame_data[5];
} SRT0_TEX_ANIM_DESC;

typedef struct
{
	uint32_t material_name_offset; /* Offset from the start of the structure */
	uint32_t texture_flags; /* M bits set */
	uint32_t unk_08;
	uint32_t* entry_offsets;
	
	SRT0_TEX_ANIM_DESC* entries;
	
	/* Helper fields, not in the format */
	char* material_name;
	uint32_t material_name_size;
	uint8_t m_bits;
} SRT0_TEX_ANIM_DATA;

typedef struct
{
	BRRES_SUB_HEADER sub_header;
	SRT0_HEADER header;
	BRRES_GROUP group;
	SRT0_TEX_ANIM_DATA* anim_data;
} SRT0_FILE;

/* Functions */

SRT0_FILE* srt0_read_file(FU_FILE* srt0);

void srt0_read_sub_header(FU_FILE* srt0, BRRES_SUB_HEADER* sub_header);
void srt0_read_header(FU_FILE* srt0, SRT0_HEADER* header);
void srt0_read_groups(FU_FILE* srt0, SRT0_FILE* srt0f);
void srt0_read_tex_anim_data(FU_FILE* srt0, SRT0_TEX_ANIM_DATA* anim_data);
void srt0_read_tex_anim_desc(FU_FILE* srt0, SRT0_TEX_ANIM_DESC* anim_desc);
void srt0_read_tex_frame_data(FU_FILE* srt0, SRT0_TEX_ANIM_FRAME_DATA* frame_data);

uint8_t srt0_count_bits32(uint32_t val);

SRT0_FILE* srt0_alloc_file();

void srt0_print(SRT0_FILE* srt0);

void srt0_free(SRT0_FILE* srt0);