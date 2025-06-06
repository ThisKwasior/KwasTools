#pragma once

/*
    CRIWARE archive for audio.
	
	Endianness on PC is little.
*/

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

#define AWB_MAGIC       (const char*)"AFS2"
#define AWB_HEADER_SIZE 16

#define AWB_DATA_BIN    0
#define AWB_DATA_ADX    1
#define AWB_DATA_HCA    2

typedef struct
{
    char* magic[4];
    uint8_t version;
    uint8_t offset_size;
    uint8_t id_size;
    uint8_t unk;
    uint32_t file_count;
    uint16_t alignment;
    uint16_t subkey;
} AWB_HEADER;

typedef struct
{
    uint32_t id;
    uint8_t* data;
    uint32_t size;
    uint32_t offset;
    
    uint8_t type;
} AWB_ENTRY;

/* Vector of AWB_ENTRY */
/* typedef CVEC AWB_FILE; */
typedef struct
{
    AWB_HEADER header;
    CVEC entries;
} AWB_FILE;

/*
    Implementation
*/

AWB_FILE* awb_alloc();

AWB_FILE* awb_free(AWB_FILE* awb);

AWB_FILE* awb_load_from_data(const uint8_t* data, const uint32_t size);

AWB_ENTRY* awb_append_entry(AWB_FILE* awb, const uint32_t id, const uint8_t* data, const uint32_t size);

SU_STRING* awb_to_data(AWB_FILE* awb);

AWB_ENTRY* awb_get_entry_by_id(AWB_FILE* awb, const uint32_t id);

const uint32_t awb_get_file_count(AWB_FILE* awb);

const uint32_t awb_fix_offset(const uint32_t offset, const uint32_t alignment);