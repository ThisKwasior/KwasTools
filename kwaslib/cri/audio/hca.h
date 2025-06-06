#pragma once

/*
    High Compression Audio - CRI ADX2
    
    File is in big endian on all platforms.
    
    HCA spec mostly borrowed from
    https://github.com/Thealexbarney/VGAudio/blob/master/docs/010-editor-templates/hca.bt
    https://github.com/kohos/CriTools/blob/master/src/hca.js
    
    Blocks are checksum'd with CRC-16/UMTS
*/

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>

#define HCA_MAGIC   (const char*)"HCA\0"

typedef struct
{
    uint8_t channel_count;
    uint32_t sample_rate : 24;
    uint32_t block_count;
    uint16_t inserted_samples;
    uint16_t appended_samples;
} HCA_SECTION_FMT;

typedef struct
{
    uint16_t block_size;
    int8_t min_res;
    int8_t max_res;
    int8_t track_count;
    int8_t channel_config;
    uint8_t total_band_count;
    uint8_t base_band_count;
    uint8_t stereo_band_count;
    uint8_t bands_per_hfr_group;
    uint8_t reserved[2];
} HCA_SECTION_COMP;

typedef struct
{
    uint16_t block_size;
    int8_t min_res;
    int8_t max_res;
    uint8_t total_band_count;
    uint8_t base_band_count;
    int8_t track_count : 4;
    int8_t channel_config : 4;
    int8_t stereo_type;
} HCA_SECTION_DEC;

typedef struct
{
    uint16_t max_frame_size;
    uint16_t noise_level;
} HCA_SECTION_VBR;

typedef struct
{
    uint16_t ath_table_type;
} HCA_SECTION_ATH;

typedef struct
{
    uint32_t start;
    uint32_t end;
    uint16_t pre_loop_samples;
    uint16_t post_loop_samples;
} HCA_SECTION_LOOP;

typedef struct
{
    uint16_t type; /* 0, 1 and 56 */
} HCA_SECTION_CIPH;

typedef struct
{
    float volume;
} HCA_SECTION_RVA;

typedef struct
{
    uint16_t fmt : 1;
    uint16_t comp : 1;
    uint16_t dec : 1;
    uint16_t vbr : 1;
    uint16_t ath : 1;
    uint16_t loop : 1;
    uint16_t ciph : 1;
    uint16_t rva : 1;
} HCA_SECTIONS_FLAGS;

typedef struct
{
    uint8_t magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint16_t data_offset;
    
    HCA_SECTIONS_FLAGS sections;
    HCA_SECTION_FMT fmt;
    HCA_SECTION_COMP comp;
    HCA_SECTION_DEC dec;
    HCA_SECTION_VBR vbr;
    HCA_SECTION_ATH ath;
    HCA_SECTION_LOOP loop;
    HCA_SECTION_CIPH ciph;
    HCA_SECTION_RVA rva;
} HCA_HEADER;

/*
    Implementation
*/

HCA_HEADER hca_read_header_from_data(const uint8_t* data, const uint32_t size);

const uint32_t hca_get_file_size(const HCA_HEADER hcah);