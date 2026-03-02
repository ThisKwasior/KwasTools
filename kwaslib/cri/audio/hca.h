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
#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/crypto/crc16.h>

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

/*
    Reads the HCA header from data provided.
*/
HCA_HEADER hca_read_header_from_data(const uint8_t* data, const uint32_t size);

/*
    Returns the theoretical size of the HCA.
    Keep in mind the size here can be incorrect, because prefetch HCAs
    in ACBs have the same header, but are only few kb long.
*/
static inline uint32_t hca_get_file_size(const HCA_HEADER hcah)
{
    uint32_t size = 0;
    uint32_t block_size = 0;
    uint32_t block_count = 0;
    
    if(hcah.data_offset) size = hcah.data_offset;
    if(hcah.sections.fmt) block_count = hcah.fmt.block_count;
    
    if(hcah.sections.comp) block_size = hcah.comp.block_size;
    else if(hcah.sections.dec) block_size = hcah.dec.block_size;

    size += block_size*block_count;
    
    return size;
}

/*
    Checks if the hash at the end of the block matches the calculated one.
    Returns 1 if it matches, 0 otherwise.
*/
static inline uint8_t hca_check_block_hash(const uint8_t* data, const uint32_t block_size)
{
    const uint16_t hash = tr_read_u16be(&data[block_size-2]);
    const uint16_t calculated_hash = crc16_encode_umts(data, block_size-2);
    
    if(hash == calculated_hash)
        return 1;
    
    return 0;
}

/*
    Counts blocks by validating the hash of the block.
    Goes as long as the hash matches or we reached the size of the data.
*/
static inline uint32_t hca_count_valid_blocks(const uint8_t* data, const uint32_t size, const HCA_HEADER hcah)
{
    uint32_t block_size = 0;
    uint32_t block_count = 0;
    
    if(hcah.sections.comp) block_size = hcah.comp.block_size;
    else if(hcah.sections.dec) block_size = hcah.dec.block_size;
    
    const uint32_t max_blocks = (size-hcah.data_offset)/block_size;
    const uint8_t* data_ptr = (const uint8_t*)&data[hcah.data_offset];
    
    for(uint32_t i = 0; i != max_blocks; ++i)
    {
        const uint8_t check = hca_check_block_hash(&data_ptr[i*block_size], block_size);
        
        if(check)
            block_count += 1;
        else
            break;
    }
    
    return block_count;
}