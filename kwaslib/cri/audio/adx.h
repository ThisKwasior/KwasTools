#pragma once

/*
    ADX - CRIWARE's 4-bit ADPCM.
    Data is in big endian.
    
    Spec from
    https://github.com/Thealexbarney/VGAudio/blob/master/docs/010-editor-templates/adx.bt
*/

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

#define ADX_MAGIC               (const char*)"\x80\x00"
#define ADX_CRI_COPYRIGHT_STR   (const char*)"(c)CRI"

typedef struct
{
    uint8_t s1 : 4;
    uint8_t s2 : 4;
} ADX_SAMPLE_BYTE;

typedef struct
{
    uint16_t filter_num : 3;
    uint16_t scale : 13;
    CVEC samples; /* Array of ADX_SAMPLE_BYTE */
} ADX_FRAME;

typedef struct
{
    uint16_t loop_num;
    uint16_t loop_type;
    uint32_t loop_start_sample;
    uint32_t loop_start_byte;
    uint32_t loop_end_sample;
    uint32_t loop_end_byte;
} ADX_LOOP;

typedef struct
{
    int16_t hist1;
    int16_t hist2;
} ADX_HISTORY;

typedef struct
{
    uint8_t magic[2];
    uint16_t header_size;
    uint8_t encoding_type;
    uint8_t frame_size;
    uint8_t bit_depth;
    uint8_t channel_count;
    uint32_t sample_rate;
    uint32_t sample_count;
    int16_t highpass_freq;
    uint8_t version;
    uint8_t revision;
    
    uint16_t alignment_samples;
    uint16_t loop_count;
    CVEC loop; /* Array of ADX_LOOP */
    
    char cri_copyright[6]; /* (c)CRI */
    
    /* Only in version 4 */
    uint32_t pad;
    CVEC hist; /* Array of ADX_HISTORY */
    
} ADX_HEADER;

typedef struct
{
    uint8_t magic[2];
    uint16_t pad_len;
} ADX_FOOTER;

typedef struct
{
    ADX_HEADER header;
    CVEC frames; /* Array of ADX_FRAME */
    ADX_FOOTER footer;
} ADX_FILE;

/*
    Implementation
*/

/*
    Allocates the structure and all vectors.
    
    Returns a pointer to the ADX_FILE structure.
*/
ADX_FILE* adx_alloc();

/*
    Frees everything in the file.
    
    Returns NULL.
*/
ADX_FILE* adx_free(ADX_FILE* adx);

/*
    Loads ADX from data.
    
    Returns a pointer to the ADX_FILE structure.
*/
ADX_FILE* adx_load_from_data(const uint8_t* data, const uint32_t size);

/*
    
*/
const uint32_t adx_get_file_size(ADX_FILE* adx);