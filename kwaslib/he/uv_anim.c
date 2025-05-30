#include "uv_anim.h"

#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

#include "mirage.h"

UV_ANIM_FILE* uv_anim_alloc()
{
    UV_ANIM_FILE* uv = (UV_ANIM_FILE*)calloc(1, sizeof(UV_ANIM_FILE));
    
    if(uv)
    {
        uv->metadata.anim_offsets = cvec_create(sizeof(uint32_t));
        uv->entries = cvec_create(sizeof(UV_ANIM_ENTRY));
        uv->keyframes = cvec_create(sizeof(MIRAGE_KEYFRAME));
        uv->string_table = su_create_string("", 0);
    }
    
    return uv;
}

UV_ANIM_FILE* uv_anim_load_from_data(const uint8_t* data)
{
    UV_ANIM_FILE* uv = uv_anim_alloc();
    UV_ANIM_HEADER* h = &uv->header;
    UV_ANIM_METADATA* m = &uv->metadata;
    
    if(uv)
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
        
        m->material_name_offset = tr_read_u32be(&metadata_ptr[0]);
        m->texture_name_offset = tr_read_u32be(&metadata_ptr[4]);
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
        cvec_resize(uv->entries, m->anim_count);
        
        for(uint32_t i = 0; i != m->anim_count; ++i)
        {
            const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
            const uint8_t* entry_ptr = &data[offset];
            UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);

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
        mirage_read_keyframes_from_data(keyframes_ptr, h->keyframes_size, uv->keyframes);
        
        /* String table */
        const char* strtable_ptr = (const char*)&data[h->string_table_offset];
        su_insert_char(uv->string_table, 0, strtable_ptr, h->string_table_size);
    }
    
    return uv;
}

FU_FILE* uv_anim_export_to_fu(UV_ANIM_FILE* uv)
{
    UV_ANIM_HEADER* h = &uv->header;
    UV_ANIM_METADATA* m = &uv->metadata;
    
    uv_anim_update(uv);
    
    FU_FILE* data_fu = fu_alloc_file();
    fu_create_mem_file(data_fu);
    fu_change_buf_size(data_fu, UV_ANIM_HEADER_SIZE
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
    fu_write_u32(data_fu, m->material_name_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, m->texture_name_offset, FU_BIG_ENDIAN);
    fu_write_u32(data_fu, m->anim_count, FU_BIG_ENDIAN);
    
    /* Anim offsets */
    for(uint32_t i = 0; i != cvec_size(m->anim_offsets); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        fu_write_u32(data_fu, offset, FU_BIG_ENDIAN);
    }
    
    /* Anim entries */
    for(uint32_t i = 0; i != cvec_size(uv->entries); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);
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
    
    for(uint32_t i = 0; i != cvec_size(uv->keyframes); ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(uv->keyframes, i);
        fu_write_f32(data_fu, kf->index, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, kf->value, FU_BIG_ENDIAN);
    }
    
    /* Writing string table */
    fu_seek(data_fu, h->string_table_offset, FU_SEEK_SET);
    fu_write_data(data_fu, (const uint8_t*)uv->string_table->ptr,
                  h->string_table_size);
    
    return data_fu;
}

void uv_anim_update(UV_ANIM_FILE* uv)
{
    UV_ANIM_HEADER* h = &uv->header;
    UV_ANIM_METADATA* m = &uv->metadata;
    CVEC entry_sizes = cvec_create(sizeof(uint32_t));

    /* Metadata */
    m->anim_count = cvec_size(uv->entries);

    h->metadata_offset = 0x18; /* Constant */
    h->metadata_size = 3*4; /* Fixed metadata fields */
    h->metadata_size += m->anim_count*4; /* Anim offsets, calculated later */

    for(uint32_t i = 0; i != cvec_size(uv->entries); ++i)
    {
        UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);
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
    h->keyframes_size = cvec_size(uv->keyframes) * MIRAGE_KEYFRAME_SIZE;

    /* String table */
    mirage_pad_str_table(uv->string_table, 4);
    h->string_table_offset = h->keyframes_offset + h->keyframes_size;
    h->string_table_size = uv->string_table->size;
    
    /* Cleanup */
    entry_sizes = cvec_destroy(entry_sizes);
}

UV_ANIM_FILE* uv_anim_free(UV_ANIM_FILE* uv)
{
    if(uv)
    {
        for(uint32_t i = 0; i != cvec_size(uv->entries); ++i)
        {
            UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);
            entry->keyframe_sets = cvec_destroy(entry->keyframe_sets);
        }
        
        uv->metadata.anim_offsets = cvec_destroy(uv->metadata.anim_offsets);
        uv->entries = cvec_destroy(uv->entries);
        uv->keyframes = cvec_destroy(uv->keyframes);
        uv->string_table = su_free(uv->string_table);
        free(uv);
    }
    
    return NULL;
}

UV_ANIM_ENTRY* uv_anim_get_entry_by_id(CVEC entries, const uint32_t id)
{
    return (UV_ANIM_ENTRY*)cvec_at(entries, id);
}

CVEC uv_anim_calc_offsets(UV_ANIM_FILE* uv)
{
    CVEC offsets = cvec_create(sizeof(uint32_t));
    
    /* First six values are always there: 0, 4, 8, 12, 16, 20 */
    const uint32_t anims_offsets_ptr = uv->header.metadata_offset + 12;
    
    for(uint32_t i = 0; i != 6; ++i)
    {
        uint32_t temp = 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    for(uint32_t i = 0; i != cvec_size(uv->entries); ++i)
    {
        uint32_t temp = anims_offsets_ptr + 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    return offsets;
}