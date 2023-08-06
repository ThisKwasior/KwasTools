#pragma once

#include <stdint.h>

#include <kwaslib/utils/io/file_utils.h>
#include <kwaslib/utils/crypto/crc32.h>
#include <kwaslib/utils/cg/x360_fmt.h>

/*
	WTB contains both the metadata and texture data.
	
	There are also pairs of WTA/WTP files, which contain
	metadata and texture data files separately.
	
	-	On PC files are standard DDS.
	-	Xbox 360 textures don't have a header - info about the texture is
		being held in tex_info_offset in the header.
*/

/*
	XBOX 360 has a 52-byte structure that describes the
	S3TC-compressed texture.
*/
typedef struct
{
	uint32_t unk_0; /* 0x00000003 */
	uint32_t unk_4; /* 0x00000001 */
	uint32_t unk_8; /* 0x00000000 */
	uint32_t unk_C; /* 0x00000000 */
	uint32_t unk_10; /* 0x00000000 */
	uint32_t unk_14; /* 0xFFFF0000 */
	uint32_t unk_18; /* 0xFFFF0000 */
	
	uint8_t unk_19 : 2; /* Some files have it set, some don't. */
	uint8_t stride : 6; /* Multiply this value by 128 */
	uint8_t flags[3]; /* 0x000002 */
	
	uint8_t padding_A0[3]; /* 0x000000 */	
	uint8_t unk_23 : 2; /* Some files have it set, some don't. */
	uint8_t surface_fmt : 6; /* XBOX 360 surface color format. List in x360_fmt.h */
	uint32_t packed_res; /* Resolution packed as 12w.12h with last byte being 0x00 */
			
	uint16_t unk_28; /* 0x0000 */
	
	uint8_t some_fmt_again_1; /* Messes up the color on the texture when changed */
	uint8_t some_fmt_again_2; /* Messes up the color on the texture when changed */
	
	uint16_t unk_2C; /* 0x0000 */
	
	uint8_t mipmap_stuff_1; /* It shows up when renderdoc says there are mipmaps */
	uint8_t mipmap_stuff_2;	/* This too */
	
	uint8_t unk_30; /* 0 */
	uint8_t unk_31; /* Looks like only one bit is set at different places */
	uint8_t unk_32; /* Known values: 0x0A, 0x2A, 0x8A */
	uint8_t unk_33; /* 0 */
	
	/* These aren't in the structure */
	uint16_t unpacked_width;
	uint16_t unpacked_height;
	
} WTB_X360_INFO;

typedef struct
{
	uint32_t offset;
	uint32_t size;
	uint32_t unk; /* Always 0x20000020 ??? */
				  /* EDIT: It's 0x22000020 in vr_mission_img on the PC */
				  /* EDIT2: It's 0x60000020 in vr_mission_img on the XBOX */
	uint32_t id;
	
	uint8_t* data;
	
	WTB_X360_INFO x360; /* Xbox 360 structure with texture info */
} WTB_ENTRY;

typedef struct
{
	uint8_t magic[4]; /* WTB\0 */
	uint32_t unknown04;	/* Always 1 */
	uint32_t tex_count;
	uint32_t tex_offset_array_offset; /* 0x20 */
	uint32_t tex_size_offset;
	uint32_t unk_array_offset;
	uint32_t tex_id_array_offset;
	uint32_t tex_info_offset; /* This is an offset in the X360 version.
								 Points to 52-byte structures with info for
								 decoding S3TC-compressed and raw images */
} WTB_HEADER;

typedef struct
{
	WTB_HEADER header;
	WTB_ENTRY* entries;
	
	uint8_t platform; /* 1 for PC, 2 for X360 */
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