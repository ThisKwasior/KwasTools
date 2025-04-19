#pragma once

#include <stdint.h>

#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/crypto/crc32.h>
#include <kwaslib/core/data/image/x360_texture.h>

/*
    WTB contains both the metadata and texture data.
    
    There are also pairs of WTA/WTP files, which contain
    metadata and texture data separately.
    
    -   On PC, files are standard DDS.
    -   Xbox 360 textures don't have a header - info about the texture is
        being held in xpr_info_offset in the header.
        Data itself is an X360 D3DBaseTexture.
    -   PS3 textures are GTF resources.
*/

#define WTB_PLATFORM_INVALID 0
#define WTB_PLATFORM_LE FU_LITTLE_ENDIAN
#define WTB_PLATFORM_BE FU_BIG_ENDIAN

#define WTB_MAGIC_LE (const char*)"WTB\0"
#define WTB_MAGIC_BE (const char*)"\0BTW"

#define WTB_SECTION_ALIGNMENT 32
#define WTB_BLOCK_SIZE 0x1000

typedef union
{
    struct
    {
        uint32_t f8_1 : 1;
        uint32_t noncomplex : 1;    /* DDSCAPS_COMPLEX ? 0 : 1 */
        uint32_t f6_1 : 1;
        uint32_t f5_1 : 1;
        uint32_t f4_1 : 1;
        uint32_t always_set : 1;    /* 1, could be a texture flag */
        uint32_t f2_1 : 1;
        uint32_t f1_1 : 1;

        uint32_t pad : 16;

        uint32_t f8_2 : 1;
        uint32_t f7_2 : 1;
        uint32_t f6_2 : 1;
        uint32_t alphaonly : 1;     /* DDPF_ALPHA */
        uint32_t dxt1a : 1;         /* DXT1 with 1-bit alpha, seems to do nothing */
        uint32_t atlas : 1;         /* Texture atlas ? seems to do nothing */
        uint32_t f2_2 : 1;
        uint32_t cubemap : 1;       /* DDSCAPS2_CUBEMAP */
    } data;
    
    uint32_t buf;
} WTB_FLAGS;

typedef struct
{
    uint32_t position;
    uint32_t size;
    WTB_FLAGS flags;
    uint32_t id;
    uint8_t* data;
    D3DBaseTexture x360;
} WTB_ENTRY;

typedef struct
{
    uint8_t magic[4]; /* WTB\0 for LE, \0BTW for BE*/
    uint32_t version;    /* Always 1 */
    uint32_t tex_count;
    uint32_t positions_offset; /* Sections are aligned to 0x20 */
    uint32_t sizes_offset;
    uint32_t flags_offset;
    uint32_t ids_offset;
    uint32_t xpr_info_offset; /* X360 exclusive. Points to an array of D3DBaseTexture */
} WTB_HEADER;

typedef struct
{
    WTB_HEADER header;
    CVEC entries;
    
    uint8_t platform; /* 1 for PC, 2 for X360/PS3 */
} WTB_FILE;

/*
    Implementation
*/

WTB_FILE* wtb_parse_wta_wtp(FU_FILE* fwta, FU_FILE* fwtp);
WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb);

FU_FILE* wtb_header_to_fu_file(WTB_FILE* wtb);
FU_FILE* wtb_data_to_fu_file(WTB_FILE* wtb);

/*
    Updates header offsets and positions of the image data.
*/
void wtb_update(WTB_FILE* wtb, const uint8_t wtp);

/*
    Allocates the structure and sets platform to PC.
    Return a pointer.
*/
WTB_FILE* wtb_alloc_wtb();

/*
    Frees the contents and the pointer.
    Returns NULL.
*/
WTB_FILE* wtb_free(WTB_FILE* wtb);

/*
    Reads the WTB header.
*/
void wtb_read_header(FU_FILE* fwtb, WTB_FILE* wtb);

/*
    Reads the image data.
*/
void wtb_read_image_data(FU_FILE* fwtb, WTB_FILE* wtb);

/*
    Appends new entry
*/
void wtb_append_entry(CVEC entries,
                      const uint32_t size,
                      const uint32_t id,
                      const uint8_t* data);

/*
    Returns a pointer to a WTB_ENTRY.
*/
WTB_ENTRY* wtb_get_entry_by_id(CVEC entries, const uint64_t id);

/*
    Will set texture flags from the DDS texture.
    Atlas needs to be specified because you can't just
    get that from dds header or image data.
    
    If the file is NOT a DDS, but rather a PNG/JPEG/TIFF,
    it will set the closest flags that would be for
    a 2D RGB texture without mipmaps.
*/
void wtb_set_entry_flags(WTB_ENTRY* entry, const uint8_t atlas);