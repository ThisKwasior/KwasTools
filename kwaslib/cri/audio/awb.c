#include "awb.h"

#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/type_writers.h>
#include <kwaslib/core/math/boundary.h>
#include <kwaslib/cri/audio/hca.h>
#include <kwaslib/cri/audio/adx.h>

#include <kwaslib/nw4r/bcwav.h>

AWB_FILE* awb_alloc()
{
    AWB_FILE* awb = (AWB_FILE*)calloc(1, sizeof(AWB_FILE));
    awb->entries = cvec_create(sizeof(AWB_ENTRY));
    return awb;
}

AWB_FILE* awb_free(AWB_FILE* awb)
{
    for(uint32_t i = 0; i != cvec_size(awb->entries); ++i)
    {
        AWB_ENTRY* entry = awb_get_entry_by_id(awb, i);
        free(entry->data);
    }
    
    awb->entries = cvec_destroy(awb->entries);
    free(awb);
    return NULL;
}

AWB_FILE* awb_load_from_data(const uint8_t* data, const uint32_t size)
{
    if(su_cmp_char((const char*)data, 4, AWB_MAGIC, 4) != 0)
    {
        return NULL;
    }
    
    AWB_FILE* awb = awb_alloc();
    AWB_HEADER* h = &awb->header;
    
    if(size < sizeof(AWB_HEADER))
    {
        /* No data */
        awb_free(awb);
        return NULL;
    }
    
    /* Header */
    tr_read_array(&data[0], 4, (uint8_t*)&h->magic[0]);
    h->version = data[4];
    h->offset_size = data[5];
    h->id_size = data[6];
    h->unk = data[7];
    h->file_count = tr_read_u32le(&data[8]);
    h->alignment = tr_read_u16le(&data[12]);
    h->subkey = tr_read_u16le(&data[14]);
    
    /* File data */
    cvec_resize(awb->entries, h->file_count);
    const uint32_t ids_offset = 16;
    const uint32_t offsets_offset = ids_offset + h->file_count*h->id_size;
    const uint32_t file_size_offset = offsets_offset + h->file_count*h->offset_size;
    const uint32_t file_size = tr_read_u32le(&data[file_size_offset]);

    for(uint32_t i = 0; i != h->file_count; ++i)
    {
        AWB_ENTRY* entry = awb_get_entry_by_id(awb, i);
        const uint32_t id_pos = ids_offset + h->id_size*i;
        const uint32_t offset_pos = offsets_offset + h->offset_size*i;
        
        if(h->id_size == 2) entry->id = tr_read_u16le(&data[id_pos]);
        else if(h->id_size == 4) entry->id = tr_read_u32le(&data[id_pos]);
        
        if(h->offset_size == 2) entry->offset = tr_read_u16le(&data[offset_pos]);
        else if(h->offset_size == 4) entry->offset = tr_read_u32le(&data[offset_pos]);
        
        /* Offsets are almost never at the start of audio data */
        const uint32_t fixed_offset = awb_fix_offset(entry->offset, h->alignment);
        const uint32_t temp_size = file_size-fixed_offset;
        const uint8_t* data_offset = &data[fixed_offset];
        entry->type = AWB_DATA_BIN;
        
        /* Getting the file size of the ADX file */
        ADX_FILE* adx = adx_load_from_data(data_offset, temp_size);
        
        if(adx)
        {
            entry->type = AWB_DATA_ADX;
            entry->size = adx_get_file_size(adx);
        }
        
        /* ADX failed. Next is HCA. */
        HCA_HEADER hca = hca_read_header_from_data(data_offset, temp_size);
        
        if(hca.sections.comp || hca.sections.dec)
        {
            entry->type = AWB_DATA_HCA;
            entry->size = hca_get_file_size(hca);
            
            uint32_t sixth_block_pos = hca.data_offset;
            if(hca.sections.comp) sixth_block_pos += 6*hca.comp.block_size;
            if(hca.sections.dec) sixth_block_pos += 6*hca.dec.block_size;
            const uint32_t block_start_offset = fixed_offset+sixth_block_pos;
            
            /*
                We've gone past the boundary of the AFS2 archive.
                The check would crash the program later.
                It's either very small hca or prefetch one.
                TODO: Do it better. What if block count is smaller than 6?
            */
            if(block_start_offset >= size)
            {
                hca.fmt.block_count = 5;
                entry->size = hca_get_file_size(hca);
            }
            else
            {
                const uint16_t block_start_value = tr_read_u16be(&data[block_start_offset]);
                
                /*if(entry->size >= file_size)*/
                if(block_start_value != 0xFFFF)
                {
                    /* It's probably a prefetch hca */
                    hca.fmt.block_count = 5;
                    entry->size = hca_get_file_size(hca);
                }
            }
        }
        
        /* N3DS format used in Lost World */
        const BCWAV_HEADER bcwav = bcwav_read_header_from_data(data_offset, temp_size);
        
        if(bcwav.header_size == BCWAV_HEADER_SIZE)
        {
            entry->type = AWB_DATA_BCWAV;
            entry->size = bcwav.file_size;
        }
        
        /* Not ADX or HCA or BCWAV */
        if(entry->type == AWB_DATA_BIN)
        {
            entry->size = temp_size;
        }
        
        /* Read the file data */
        entry->data = (uint8_t*)calloc(1, entry->size);
        tr_read_array(data_offset, entry->size, &entry->data[0]);
    }
    
    return awb;
}

AWB_ENTRY* awb_append_entry(AWB_FILE* awb, const uint32_t id, const uint8_t* data, const uint32_t size)
{
    AWB_ENTRY entry = {0};
    entry.id = id;
    entry.data = (uint8_t*)calloc(1, size);
    memcpy(entry.data, data, size);
    entry.size = size;
    
    cvec_push_back(awb->entries, &entry);
    return awb_get_entry_by_id(awb, cvec_size(awb->entries)-1);
}

SU_STRING* awb_to_data(AWB_FILE* awb)
{
    AWB_HEADER* h = &awb->header;
    
    const uint32_t entries_count = cvec_size(awb->entries);
    const uint32_t header_size = AWB_HEADER_SIZE
                                 + (h->offset_size+h->id_size)*entries_count
                                 + 4;
    
    uint32_t data_size = header_size;
    
    /* Recalculating offsets and file size */
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        AWB_ENTRY* entry = awb_get_entry_by_id(awb, i);
        entry->offset = data_size;
        data_size = awb_fix_offset(entry->offset, h->alignment);
        data_size += entry->size;
    }
    
    data_size = awb_fix_offset(data_size, h->alignment);

    /* Data buffer */
    SU_STRING* afs2_data = su_create_string(NULL, data_size);
    uint8_t* data = (uint8_t*)&afs2_data->ptr[0];
    
    /* Writing header */
    tw_write_array((const uint8_t*)AWB_MAGIC, 4, &data[0]);
    data[4] = h->version;
    data[5] = h->offset_size;
    data[6] = h->id_size;
    data[7] = h->unk;
    tw_write_u32le(entries_count, &data[8]);
    tw_write_u16le(h->alignment, &data[12]);
    tw_write_u16le(h->subkey, &data[14]);
    
    /* Writing ids, offsets and file data */
    const uint32_t ids_offset = 16;
    const uint32_t offsets_offset = ids_offset + entries_count*h->id_size;
    const uint32_t file_size_offset = offsets_offset + entries_count*h->offset_size;
    
    tw_write_u32le(data_size, &data[file_size_offset]);
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        AWB_ENTRY* entry = awb_get_entry_by_id(awb, i);
        const uint32_t id_pos = ids_offset + h->id_size*i;
        const uint32_t offset_pos = offsets_offset + h->offset_size*i;
        const uint32_t file_data_pos = awb_fix_offset(entry->offset, h->alignment);
        
        if(h->id_size == 2) tw_write_u16le(entry->id, &data[id_pos]);
        else if(h->id_size == 4) tw_write_u32le(entry->id, &data[id_pos]);;
        
        if(h->offset_size == 2) tw_write_u16le(entry->offset, &data[offset_pos]);
        else if(h->offset_size == 4) tw_write_u32le(entry->offset, &data[offset_pos]);
        
        tw_write_array(entry->data, entry->size, &data[file_data_pos]);
    }
    
    return afs2_data;
}

AWB_ENTRY* awb_get_entry_by_id(AWB_FILE* awb, const uint32_t id)
{
    return (AWB_ENTRY*)cvec_at(awb->entries, id);
}

const uint32_t awb_get_file_count(AWB_FILE* awb)
{
    return cvec_size(awb->entries);
}

const uint32_t awb_fix_offset(const uint32_t offset, const uint32_t alignment)
{
    const uint32_t add = bound_calc_leftover(alignment, offset);
    return offset+add;
}