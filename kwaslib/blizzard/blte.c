#include "blte.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

#include <kwaslib/ext/miniz.h>

BLTE_FILE* blte_read_file(const uint8_t* data)
{
    BLTE_FILE* blte = (BLTE_FILE*)calloc(1, sizeof(BLTE_FILE));
    
    if(blte)
    {
        blte->header = blte_read_header(data);
        
        if(blte->header.header_size != (-1))
        {
            blte->chunkinfo = blte_read_chunkinfo(&data[8]);
            blte->data = blte_read_data(&blte->chunkinfo, &data[blte->header.header_size]);
        }
        else
        {
            free(blte);
            return NULL;
        }
    }
    
    return blte;
}

BLTE_FILE* blte_free(BLTE_FILE* blte)
{
    blte->chunkinfo.blocks = cvec_destroy(blte->chunkinfo.blocks);
    
    if(blte->data)
    {
        free(blte->data);
    }
    
    return NULL;
}

BLTE_HEADER blte_read_header(const uint8_t* data)
{
    BLTE_HEADER header = {0};
    header.header_size = -1;
    
    if(memcmp(&data[0], BLTE_MAGIC, 4) == 0)
    {
        strncpy(&header.magic[0], BLTE_MAGIC, 4);
        header.header_size = tr_read_u32be(&data[4]);
    }
    
    return header;
}

BLTE_CHUNKINFO blte_read_chunkinfo(const uint8_t* data)
{
    BLTE_CHUNKINFO chunk = {0};
    
    chunk.table_fmt = data[0];
    
    if((chunk.table_fmt != BLTE_TABLE_FMT_DEFAULT)
       && (chunk.table_fmt != BLTE_TABLE_FMT_AVOWED))
    {
       return chunk;
    }
    
    chunk.num_blocks = tr_read_u32be(&data[0]);
    
    chunk.blocks = cvec_create(sizeof(BLTE_BLOCK));
    cvec_reserve(chunk.blocks, chunk.num_blocks);
    
    uint32_t pos = 4;
    
    for(uint32_t i = 0; i != chunk.num_blocks; ++i)
    {
        BLTE_BLOCK* cur_block = (BLTE_BLOCK*)cvec_at(chunk.blocks, i);
        
        cur_block->block_0f.raw_size = tr_read_u32be(&data[pos]);
        pos += 4;
        
        cur_block->block_0f.logical_size = tr_read_u32be(&data[pos]);
        pos += 4;
        
        tr_read_array(&data[pos], 16, &cur_block->block_0f.hash[0]);
        pos += 16;

        if(chunk.table_fmt == BLTE_TABLE_FMT_AVOWED)
        {
            tr_read_array(&data[pos], 16, &cur_block->block_10.logical_hash[0]);
            pos += 16;
        }
    }
    
    return chunk;
}

uint8_t* blte_read_data(const BLTE_CHUNKINFO* const chunkinfo, const uint8_t* data)
{
    const uint64_t raw_size = blte_get_raw_data_size(chunkinfo);
    
    uint8_t* out = (uint8_t*)calloc(1, raw_size);
    
    if(out)
    {
        memcpy(&out[0], &data[0], raw_size);
    }
    
    return out;
}

uint8_t* blte_data_to_raw(const BLTE_FILE* const blte)
{
    const uint64_t raw_size = blte_get_raw_data_size(&blte->chunkinfo);
    
    uint8_t* out = (uint8_t*)calloc(1, raw_size);

    uint64_t data_pos = 0;
    uint64_t out_pos = 0;

    if(out)
    {
        for(uint32_t i = 0; i != blte->chunkinfo.num_blocks; ++i)
        {
            BLTE_BLOCK* cur_block = (BLTE_BLOCK*)cvec_at(blte->chunkinfo.blocks, i);

            memcpy(&out[out_pos], &blte->data[data_pos+1], cur_block->block_0f.raw_size-1);

            out_pos += cur_block->block_0f.raw_size-1;
            data_pos += cur_block->block_0f.raw_size;
        }
    }

    return out;
}

uint8_t* blte_data_to_logical(const BLTE_FILE* const blte, uint64_t* out_size)
{
    /* Im assuming all blocks use the same encoding as the first one */
    const char encoding = blte->data[0];
    
    char* out = NULL;

    uint64_t data_pos = 0;
    uint64_t out_pos = 0;
    
    switch(encoding)
    {
        case BLTE_ENCODING_PLAIN:
            (*out_size) =  blte_get_logical_data_size(&blte->chunkinfo);
            out = blte_data_to_raw(blte);
            break;
        
        case BLTE_ENCODING_ZLIB:
            (*out_size) =  blte_get_logical_data_size(&blte->chunkinfo);
            out = (uint8_t*)calloc(1, (*out_size));
            
            for(uint32_t i = 0; i != blte->chunkinfo.num_blocks; ++i)
            {
                BLTE_BLOCK* cur_block = (BLTE_BLOCK*)cvec_at(blte->chunkinfo.blocks, i);

                //memcpy(&out[out_pos], &blte->data[data_pos+1], cur_block->block_0f.raw_size-1);
                mz_ulong block_size_raw = cur_block->block_0f.raw_size-1;
                mz_ulong block_size_logical = cur_block->block_0f.logical_size;
                mz_uncompress2(&out[out_pos], &block_size_logical, &blte->data[data_pos+1], &block_size_raw);

                out_pos += cur_block->block_0f.logical_size;
                data_pos += cur_block->block_0f.raw_size;
            }
            
            break;
        case BLTE_ENCODING_LZ4:
        case BLTE_ENCODING_RECURSIVE:
        case BLTE_ENCODING_CRYPT:
            (*out_size) =  blte_get_raw_data_size(&blte->chunkinfo);
            out = blte_data_to_raw(blte);
            break;
    }
    
    return out;
}