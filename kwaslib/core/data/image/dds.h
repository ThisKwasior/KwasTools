#pragma once

/*
    https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
*/

#include <stdint.h>

typedef union
{
    struct
    {
        uint32_t alphapixels : 1;   /* Contains alpha. dwRGBAlphaBitMask has data */
        uint32_t alpha : 1;         /* Used in older DDS files. Alpha only raw data. dwRGBBitCount/dwABitMask has data */
        uint32_t fourcc : 1;        /* Compressed RGB data. dwFourCC has data */
        uint32_t unused_1 : 3;
        uint32_t rgb : 1;           /* Uncompressed RGB data. dwRGBBitCount and masks have valid data */
        uint32_t unused_2 : 2;
        uint32_t yuv : 1;           /* YUV uncompressed data (dwRGBBitCount contains the YUV bit count; 
                                       dwRBitMask contains the Y mask, dwGBitMask contains the U mask,
                                       dwBBitMask contains the V mask) */
        uint32_t unused_3 : 7;
        uint32_t luminance : 1;     /* Single channel color uncompressed data
                                       (dwRGBBitCount contains the luminance channel bit count;
                                       dwRBitMask contains the channel mask).
                                       Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file. */
        uint32_t unused_4 : 14;
    } data;
    
    uint32_t buf;
} DDS_PIXELFORMAT_FLAGS;

typedef struct
{
    uint32_t size;
    DDS_PIXELFORMAT_FLAGS flags;
    uint8_t fourcc[4];
    uint32_t rgb_bit_count;
    uint32_t r_bitmask;
    uint32_t g_bitmask;
    uint32_t b_bitmask;
    uint32_t a_bitmask;
} DDS_PIXELFORMAT;

typedef union
{
    struct
    {
        uint32_t unused_1 : 3;
        uint32_t complex : 1;   /* Optional; must be used on any file that contains more than one surface
                                   (a mipmap, a cubic environment map, or mipmapped volume texture). */
        uint32_t unused_2 : 8;
        uint32_t texture : 1;   /* Required */
        uint32_t unused_3 : 9;
        uint32_t mipmap : 1;    /* Optional; should be used for a mipmap. */
        uint32_t unused_4 : 9;
    } data;
    
    uint32_t buf;
} DDS_CAPS;

typedef union
{
    struct
    {
        uint32_t unused_1 : 9;
        uint32_t cubemap : 1;
        uint32_t cubemap_positivex : 1;
        uint32_t cubemap_negativex : 1;
        uint32_t cubemap_positivey : 1;
        uint32_t cubemap_negativey : 1;
        uint32_t cubemap_positivez : 1;
        uint32_t cubemap_negativez : 1;
        uint32_t unused_2 : 5;
        uint32_t volume : 1;
    } data;
    
    uint32_t buf;
} DDS_CAPS2;

typedef union
{
    struct
    {
        uint32_t caps : 1;          /* Required */
        uint32_t height : 1;        /* Required */
        uint32_t width : 1;         /* Required */
        uint32_t pitch : 1;         /* Required when pitch is provided for an uncompressed texture */
        uint32_t unused_1 : 8;
        uint32_t pixelformat : 1;   /* Required */
        uint32_t unused_2 : 4;
        uint32_t mipmapcount : 1;   /* Required in a mipmapped texture */
        uint32_t unused_3 : 1;
        uint32_t linearsize : 1;    /* Required when pitch is provided for a compressed texture. */
        uint32_t unused_4 : 3;
        uint32_t depth : 1;         /* Required in a depth texture */
        uint32_t unused_5 : 8;
    } data;
    
    uint32_t buf;
} DDS_HEADER_FLAGS;

typedef struct
{
    uint32_t size;                  /* Size of structure. Has to be 124. */
    DDS_HEADER_FLAGS flags;         /* Flags to indicate which members contain valid data. */
    uint32_t height;                /* Height in pixels */
    uint32_t width;                 /* Width in pixels */
    uint32_t pitch_or_linear_size;  /* The pitch or number of bytes per scan line in an uncompressed texture */
    uint32_t depth;                 /* Depth of a volume texture (in pixels), otherwise unused. */
    uint32_t mipmap_count;          /* Number of mipmap levels, otherwise unused. */
    uint32_t reserved[11];          /* Unused */
    DDS_PIXELFORMAT pf;             /* Pixel format */
    DDS_CAPS caps;                  /* Specifies the complexity of the surfaces stored. */
    DDS_CAPS2 caps2;                /* Additional detail about the surfaces stored. */
    uint32_t caps3;                 /* Unused */
    uint32_t caps4;                 /* Unused */
    uint32_t reserved2;             /* Unused */
} DDS_HEADER;

typedef struct
{
    uint8_t magic[4];
    DDS_HEADER header;
} DDS_FILE;

/*
    Implementation
*/

DDS_FILE* dds_header_from_data(const char* data);
void dds_read_header(DDS_FILE* dds, const char* data);
void dds_print_info(DDS_FILE* dds);