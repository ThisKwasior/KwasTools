#pragma once

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>

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

#define WTB_PLATFORM_LE 1
#define WTB_PLATFORM_BE 2

#define WTB_MAGIC_LE (const char*)"WTB\0"
#define WTB_MAGIC_BE (const char*)"\0BTW"

#define WTB_SECTION_ALIGNMENT 32
#define WTB_BLOCK_SIZE 0x1000

typedef struct
{
    struct
    {
        uint8_t unk_0x0_7 : 1;
        uint8_t complex   : 1; /* DDSCAPS_COMPLEX */
        uint8_t unk_0x0_5 : 1;
        uint8_t unk_0x0_4 : 1;
        uint8_t unk_0x0_3 : 1;
        uint8_t always2_0 : 1; /* This bit was always set */
        uint8_t unk_0x0_1 : 1;
        uint8_t unk_0x0_0 : 1;
    } b0;
    
    uint32_t b12 : 16;
    
    struct
    {
        uint8_t unk_0x3_7 : 1;
        uint8_t unk_0x3_6 : 1;
        uint8_t unk_0x3_5 : 1;
        uint8_t alpha     : 1; /* DDPF_ALPHA */
        uint8_t unk_0x3_3 : 1;
        uint8_t always2_1 : 1; /* Almost always set, a lot of ui_ textures have this byte be 0x00 */
        uint8_t unk_0x3_1 : 1;
        uint8_t cubemap   : 1; /* DDSCAPS2_CUBEMAP */
    } b3;
} WTB_FLAGS;

typedef struct
{
	uint32_t offset;
	uint32_t size;
	WTB_FLAGS flags;
	uint32_t id;
	uint8_t* data;
	D3DBaseTexture x360;
} WTB_ENTRY;

typedef struct
{
	uint8_t magic[4]; /* WTB\0 for LE, \0BTW for BE*/
	uint32_t unknown04;	/* Always 1 */
	uint32_t tex_count;
	uint32_t tex_offset_array_offset; /* Sections are aligned to 0x20 */
	uint32_t tex_size_offset;
	uint32_t flag_array_offset;
	uint32_t tex_id_array_offset;
	uint32_t xpr_info_offset; /* X360 exclusive. Points to an array of D3DBaseTexture */
} WTB_HEADER;

typedef struct
{
	WTB_HEADER header;
	CVECTOR_METADATA* entries;
	
	uint8_t platform; /* 1 for PC, 2 for X360/PS3 */
} WTB_FILE;

/*
	Functions
*/

/*
    For creating the WTB by hand.
    Allocates entries vector and sets platform to PC.
*/
WTB_FILE* wtb_alloc_empty_wtb();
WTB_FILE* wtb_free(WTB_FILE* wtb);

WTB_FILE* wtb_parse_wta_wtp(FU_FILE* fwta, FU_FILE* fwtp);
WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb);

uint8_t wtb_read_header(FU_FILE* file, WTB_FILE* wtb);
void wtb_populate_entries(FU_FILE* file, WTB_FILE* wtb);
void wtb_load_texture_data(FU_FILE* file, WTB_FILE* wtb);

/*
    Requires only size, id and data filled in the entries vector.
    Will populate everything else, including the header.
*/
void wtb_update_on_export(WTB_FILE* wtb, const uint8_t external_data);

/*
    Writes header to allocated FU_FILE
*/
void wtb_write_wta_fu(WTB_FILE* wtb, FU_FILE* fwta);

/*
    Writes dds data to FU_FILE
*/
void wtb_write_wtp_fu(WTB_FILE* wtb, FU_FILE* fwtp);

void wtb_save_wtb_to_fu_file(WTB_FILE* wtb, FU_FILE* fwtb);
void wtb_save_wta_wtp_to_fu_files(WTB_FILE* wtb, FU_FILE* fwta, FU_FILE* fwtp);

void wtb_set_flags_from_dds(WTB_FILE* wtb);