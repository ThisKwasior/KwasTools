#pragma once

#include <stdint.h>
#include <string.h>

#include <kwaslib/core/io/type_readers.h>

/*
    https://problemkaputt.de/gbatek-3ds-files-sound-wave-data-cwav-format.htm
*/

/*
    Defines
*/
#define BCWAV_MAGIC         (const char*)"CWAV"
#define BCWAV_HEADER_SIZE   (uint16_t)(0x40)

/*
    Types
*/

typedef struct
{
    char magic[4];              /* "CWAV" */
    uint16_t byte_order;
    uint16_t header_size;
    uint32_t version;
    uint32_t file_size;
    uint16_t block_count;
    uint16_t reserved;
    
    uint32_t info_block_ref_id;
    uint32_t info_block_offset;
    uint32_t info_block_size;
    
    uint32_t data_block_ref_id;
    uint32_t data_block_offset;
    uint32_t data_block_size;
    
    uint8_t padding[0x14];      /* To 32-byte boundary */
} BCWAV_HEADER;

/*
    Functions
*/
static inline const BCWAV_HEADER bcwav_read_header_from_data(const uint8_t* data, const uint32_t size)
{
    BCWAV_HEADER h = {0};

    if(size < BCWAV_HEADER_SIZE)
    {
        /* File is smaller that BCWAV header */
        return h;
    }
    
    tr_read_array(data, 4, (uint8_t*)&h.magic[0]);
    
    if(su_cmp_char(BCWAV_MAGIC, 4, h.magic, 4) != SU_STRINGS_MATCH)
    {
        /* Not CWAV */
        return h;
    }

    h.byte_order = tr_read_u16le(&data[4]);
    h.header_size = tr_read_u16le(&data[6]);
    h.version = tr_read_u32le(&data[8]);
    h.file_size = tr_read_u32le(&data[12]);
    h.block_count = tr_read_u16le(&data[16]);
    h.reserved = tr_read_u16le(&data[18]);
    
    h.info_block_ref_id = tr_read_u32le(&data[20]);
    h.info_block_offset = tr_read_u32le(&data[24]);
    h.info_block_size = tr_read_u32le(&data[28]);
    
    h.data_block_ref_id = tr_read_u32le(&data[32]);
    h.data_block_offset = tr_read_u32le(&data[36]);
    h.data_block_size = tr_read_u32le(&data[40]);

    return h;
}
