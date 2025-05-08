#include "vis_anim.h"

#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

#include "mirage.h"

VIS_ANIM_FILE* vis_anim_alloc()
{
    VIS_ANIM_FILE* vis = (VIS_ANIM_FILE*)calloc(1, sizeof(VIS_ANIM_FILE));
    
    if(vis)
    {
        vis->metadata.anim_offsets = cvec_create(sizeof(uint32_t));
        vis->entries = cvec_create(sizeof(VIS_ANIM_ENTRY));
        vis->keyframes = cvec_create(sizeof(MIRAGE_KEYFRAME));
        vis->string_table = su_create_string("", 0);
    }
    
    return vis;
}

VIS_ANIM_FILE* vis_anim_load_from_data(const uint8_t* data)
{
    VIS_ANIM_FILE* vis = vis_anim_alloc();
    VIS_ANIM_HEADER* h = &vis->header;
    VIS_ANIM_METADATA* m = &vis->metadata;
    
    if(vis)
    {
        /* Header */
        h->metadata_offset = tr_read_u32be(&data[0]);
        h->metadata_size = tr_read_u32be(&data[4]);
        h->keyframes_offset = tr_read_u32be(&data[8]);
        h->keyframes_size = tr_read_u32be(&data[12]);
        h->string_table_offset = tr_read_u32be(&data[16]);
        h->string_table_size = tr_read_u32be(&data[20]);
        
        /* Metadata */
        const uint8_t* metadata_ptr = &data[h->metadata_offset];
        
        m->model_name_offset = tr_read_u32be(&metadata_ptr[0]);
        m->unk_name_offset = tr_read_u32be(&metadata_ptr[4]);
        m->anim_count = tr_read_u32be(&metadata_ptr[8]);
        metadata_ptr += 12;
        
        m->anim_offsets = cvec_create(sizeof(uint32_t));

        if(m->anim_offsets)
        {
            cvec_resize(m->anim_offsets, m->anim_count);
            
            for(uint32_t i = 0; i != m->anim_count; ++i)
            {
                uint32_t* cur_offset = (uint32_t*)cvec_at(m->anim_offsets, i);
                *cur_offset = tr_read_u32be(metadata_ptr);
                metadata_ptr += 4;
            }
        }
        
        /* Entries */
        cvec_resize(vis->entries, m->anim_count);
        
        for(uint32_t i = 0; i != m->anim_count; ++i)
        {
            const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
            const uint8_t* entry_ptr = &data[offset];
            VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);

            entry->name_offset = tr_read_u32be(&entry_ptr[0]);
            entry->frame_rate = tr_read_f32be(&entry_ptr[4]);
            entry->start_frame = tr_read_f32be(&entry_ptr[8]);
            entry->end_frame = tr_read_f32be(&entry_ptr[12]);
            entry->keyframe_set_count = tr_read_u32be(&entry_ptr[16]);
            
            entry_ptr += 20;
            entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
            mirage_read_keyframe_sets_from_data(entry_ptr,
                                                entry->keyframe_set_count,
                                                entry->keyframe_sets);
        }
        
        /* Keyframes */
        const uint8_t* keyframes_ptr = &data[h->keyframes_offset];
        mirage_read_keyframes_from_data(keyframes_ptr, h->keyframes_size, vis->keyframes);
        
        /* String table */
        const char* strtable_ptr = (const char*)&data[h->string_table_offset];
        su_insert_char(vis->string_table, 0, strtable_ptr, h->string_table_size);
    }
    
    return vis;
}

FU_FILE* vis_anim_export_to_fu(VIS_ANIM_FILE* vis)
{
    VIS_ANIM_HEADER* h = &vis->header;
    VIS_ANIM_METADATA* m = &vis->metadata;
    
    vis_anim_update(vis);
    
    FU_FILE* data_fu = fu_alloc_file();
    fu_create_mem_file(data_fu);
    fu_change_buf_size(data_fu, VIS_ANIM_HEADER_SIZE
                            + h->metadata_size
                            + h->keyframes_size
                            + h->string_table_size);
                            
    /* Writing header */
    fu_seek(data_fu, 0, FU_SEEK_SET);
    fu_write_u32(data_fu, h->metadata_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, h->metadata_size, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, h->keyframes_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, h->keyframes_size, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, h->string_table_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, h->string_table_size, FU_BIG_ENDIAN);
    
    /* Writing metadata */
    fu_seek(data_fu, h->metadata_offset, FU_SEEK_SET);
    fu_write_u32(data_fu, m->model_name_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, m->unk_name_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, m->anim_count, FU_BIG_ENDIAN);
    
    /* Anim offsets */
    for(uint32_t i = 0; i != cvec_size(m->anim_offsets); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        fu_write_u32(data_fu, offset, FU_BIG_ENDIAN);
    }
    
    /* Anim entries */
    for(uint32_t i = 0; i != cvec_size(vis->entries); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);
        fu_seek(data_fu, offset, FU_SEEK_SET);
        fu_write_u32(data_fu, entry->name_offset, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->frame_rate, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->start_frame, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->end_frame, FU_BIG_ENDIAN);
        fu_write_u32(data_fu, entry->keyframe_set_count, FU_BIG_ENDIAN);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != cvec_size(entry->keyframe_sets); ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            fu_write_u8(data_fu, kfs->type);
            fu_write_u8(data_fu, kfs->flag2);
            fu_write_u8(data_fu, kfs->interpolation);
            fu_write_u8(data_fu, kfs->flag4);
            fu_write_u32(data_fu, kfs->length, FU_BIG_ENDIAN);
            fu_write_u32(data_fu, kfs->start, FU_BIG_ENDIAN);
        }
    }
    
    /* Writing keyframes */
    fu_seek(data_fu, h->keyframes_offset, FU_SEEK_SET);
    
    for(uint32_t i = 0; i != cvec_size(vis->keyframes); ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(vis->keyframes, i);
        fu_write_f32(data_fu, kf->index, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, kf->value, FU_BIG_ENDIAN);
    }
    
    /* Writing string table */
    fu_seek(data_fu, h->string_table_offset, FU_SEEK_SET);
    fu_write_data(data_fu, (const uint8_t*)vis->string_table->ptr,
                  h->string_table_size);
    
    return data_fu;
}

void vis_anim_update(VIS_ANIM_FILE* vis)
{
    VIS_ANIM_HEADER* h = &vis->header;
    VIS_ANIM_METADATA* m = &vis->metadata;
    CVEC entry_sizes = cvec_create(sizeof(uint32_t));

    /* Metadata */
    m->anim_count = cvec_size(vis->entries);

    h->metadata_offset = 0x18; /* Constant */
    h->metadata_size = 3*4; /* Fixed metadata fields */
    h->metadata_size += m->anim_count*4; /* Anim offsets, calculated later */

    for(uint32_t i = 0; i != cvec_size(vis->entries); ++i)
    {
        VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);
        uint32_t entry_size = 5*4 + entry->keyframe_set_count*MIRAGE_KEYFRAME_SET_SIZE;
        h->metadata_size += entry_size;
        cvec_push_back(entry_sizes, &entry_size);
    }

    /* Anim offsets */
    cvec_resize(m->anim_offsets, m->anim_count);

    /* anim offsets array starts at const offset */
    uint32_t anim_it = 0x24 + m->anim_count*4;

    for(uint32_t i = 0; i != m->anim_count; ++i)
    {
        uint32_t* offset = (uint32_t*)cvec_at(m->anim_offsets, i);
        const uint32_t cur_entry_size = *(uint32_t*)cvec_at(entry_sizes, i);
        *offset = anim_it;
        anim_it += cur_entry_size;
    }

    /* Keyframes */
    h->keyframes_offset = h->metadata_offset + h->metadata_size;
    h->keyframes_size = cvec_size(vis->keyframes) * MIRAGE_KEYFRAME_SIZE;

    /* String table */
    mirage_pad_str_table(vis->string_table, 4);
    h->string_table_offset = h->keyframes_offset + h->keyframes_size;
    h->string_table_size = vis->string_table->size;
    
    /* Cleanup */
    entry_sizes = cvec_destroy(entry_sizes);
}

VIS_ANIM_FILE* vis_anim_free(VIS_ANIM_FILE* vis)
{
    if(vis)
    {
        for(uint32_t i = 0; i != cvec_size(vis->entries); ++i)
        {
            VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);
            entry->keyframe_sets = cvec_destroy(entry->keyframe_sets);
        }
        
        vis->metadata.anim_offsets = cvec_destroy(vis->metadata.anim_offsets);
        vis->entries = cvec_destroy(vis->entries);
        vis->keyframes = cvec_destroy(vis->keyframes);
        vis->string_table = su_free(vis->string_table);
        free(vis);
    }
    
    return NULL;
}

VIS_ANIM_ENTRY* vis_anim_get_entry_by_id(CVEC entries, const uint32_t id)
{
    return (VIS_ANIM_ENTRY*)cvec_at(entries, id);
}

CVEC vis_anim_calc_offsets(VIS_ANIM_FILE* vis)
{
    CVEC offsets = cvec_create(sizeof(uint32_t));
    
    /* First six values are always there: 0, 4, 8, 12, 16, 20 */
    const uint32_t anims_offsets_ptr = vis->header.metadata_offset + 12;
    
    for(uint32_t i = 0; i != 6; ++i)
    {
        uint32_t temp = 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    for(uint32_t i = 0; i != cvec_size(vis->entries); ++i)
    {
        uint32_t temp = anims_offsets_ptr + 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    return offsets;
}