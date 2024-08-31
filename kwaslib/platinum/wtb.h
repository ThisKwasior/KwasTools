#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/crypto/crc32.h>
#include <kwaslib/core/data/image/x360_texture.h>

/*
	WTB contains both the metadata and texture data.
	
	There are also pairs of WTA/WTP files, which contain
	metadata and texture data files separately.
	
	-	On PC files are standard DDS.
	-	Xbox 360 textures don't have a header - info about the texture is
		being held in xpr_info_offset in the header.
		Data itself is an X360 D3DBaseTexture.
	-	PS3 textures are GTF resources.
*/

typedef struct
{
	uint32_t offset;
	uint32_t size;
	uint32_t unk; /* Always 0x20000020 ??? */
				  /* EDIT: It's 0x22000020 in vr_mission_img on the PC */
				  /* EDIT2: It's 0x60000020 in vr_mission_img on the XBOX */
	uint32_t id;
	
	uint8_t* data;
	
	D3DBaseTexture x360;
} WTB_ENTRY;

typedef struct
{
	uint8_t magic[4]; /* WTB\0 for LE, \0BTW for BE*/
	uint32_t unknown04;	/* Always 1 */
	uint32_t tex_count;
	uint32_t tex_offset_array_offset; /* 0x20 */
	uint32_t tex_size_offset;
	uint32_t unk_array_offset;
	uint32_t tex_id_array_offset;
	uint32_t xpr_info_offset; /* X360 exclusive. Points to an array of D3DBaseTexture */
} WTB_HEADER;

typedef struct
{
	WTB_HEADER header;
	WTB_ENTRY* entries;
	
	uint8_t platform; /* 1 for PC, 2 for X360/PS3 */
} WTB_FILE;

/*
	Functions
*/

WTB_FILE* wtb_parse_wta_wtp(FU_FILE* fwta, FU_FILE* fwtp);
WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb);

WTB_FILE* wtb_parse_directory(const char* dir);

void wtb_save_wtb_to_fu_file(WTB_FILE* wtb, FU_FILE* fwtb);
void wtb_save_wta_wtp_to_fu_files(WTB_FILE* wtb, FU_FILE* fwta, FU_FILE* fwtp);

uint8_t wtb_read_header(FU_FILE* file, WTB_FILE* wtb);
void wtb_populate_entries(FU_FILE* file, WTB_FILE* wtb);
void wtb_load_entries_dds(FU_FILE* file, WTB_FILE* wtb);

void wtb_free(WTB_FILE* wtb);