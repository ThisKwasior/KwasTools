#include "lit_anim.h"

#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

#include "mirage.h"

LIT_ANIM_FILE* lit_anim_alloc()
{
    LIT_ANIM_FILE* lit = (LIT_ANIM_FILE*)calloc(1, sizeof(LIT_ANIM_FILE));
    
    if(lit)
    {
        lit->metadata.anim_offsets = cvec_create(sizeof(uint32_t));
        lit->entries = cvec_create(sizeof(LIT_ANIM_ENTRY));
        lit->keyframes = cvec_create(sizeof(MIRAGE_KEYFRAME));
        lit->string_table = su_create_string("", 0);
    }
    
    return lit;
}

LIT_ANIM_FILE* lit_anim_load_from_data(const uint8_t* data)
{
    LIT_ANIM_FILE* lit = lit_anim_alloc();
    LIT_ANIM_HEADER* h = &lit->header;
    LIT_ANIM_METADATA* m = &lit->metadata;
    
    if(lit)
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
        m->anim_count = tr_read_u32be(&metadata_ptr[0]);
        metadata_ptr += 4;
        
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
        cvec_resize(lit->entries, m->anim_count);
        
        for(uint32_t i = 0; i != m->anim_count; ++i)
        {
            const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
            const uint8_t* entry_ptr = &data[offset];
            LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);

            entry->name_offset = tr_read_u32be(&entry_ptr[0]);
            entry->light_type = tr_read_u8(&entry_ptr[4]);
            entry->attribute = tr_read_u8(&entry_ptr[5]);
            entry->frame_rate = tr_read_f32be(&entry_ptr[8]);
            entry->start_frame = tr_read_f32be(&entry_ptr[12]);
            entry->end_frame = tr_read_f32be(&entry_ptr[16]);
            entry->keyframe_set_count = tr_read_u32be(&entry_ptr[20]);
            
            entry->unk_00 = tr_read_f32be(&entry_ptr[24]);
            entry->unk_01 = tr_read_f32be(&entry_ptr[28]);
            entry->unk_02 = tr_read_f32be(&entry_ptr[32]);
            entry->unk_03 = tr_read_f32be(&entry_ptr[36]);
            
            entry->color_red = tr_read_f32be(&entry_ptr[40]);
            entry->color_green = tr_read_f32be(&entry_ptr[44]);
            entry->color_blue = tr_read_f32be(&entry_ptr[48]);
            entry->unk_07 = tr_read_f32be(&entry_ptr[52]);
            
            entry->unk_08 = tr_read_f32be(&entry_ptr[56]);
            entry->unk_09 = tr_read_f32be(&entry_ptr[60]);
            entry->unk_0A = tr_read_f32be(&entry_ptr[64]);
            entry->unk_0B = tr_read_f32be(&entry_ptr[68]);
            
            entry->unk_0C = tr_read_f32be(&entry_ptr[72]);
            entry->unk_0D = tr_read_f32be(&entry_ptr[76]);
            entry->unk_0E = tr_read_f32be(&entry_ptr[80]);
            entry->intensity = tr_read_f32be(&entry_ptr[84]);
            
            entry->unk_10 = tr_read_f32be(&entry_ptr[88]);
            entry->unk_11 = tr_read_f32be(&entry_ptr[92]);
            entry->unk_12 = tr_read_f32be(&entry_ptr[96]);
            entry->unk_13 = tr_read_f32be(&entry_ptr[100]);
            
            entry->unk_14 = tr_read_f32be(&entry_ptr[104]);
            entry->unk_15 = tr_read_f32be(&entry_ptr[108]);
            entry->unk_16 = tr_read_f32be(&entry_ptr[112]);
            entry->unk_17 = tr_read_f32be(&entry_ptr[116]);
            
            entry->unk_18 = tr_read_f32be(&entry_ptr[120]);
            entry->unk_19 = tr_read_f32be(&entry_ptr[124]);
            entry->unk_1A = tr_read_f32be(&entry_ptr[128]);
            entry->unk_1B = tr_read_f32be(&entry_ptr[132]);
            
            entry_ptr += 136;
            entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
            mirage_read_keyframe_sets_from_data(entry_ptr,
                                                entry->keyframe_set_count,
                                                entry->keyframe_sets);
        }
        
        /* Keyframes */
        const uint8_t* keyframes_ptr = &data[h->keyframes_offset];
        mirage_read_keyframes_from_data(keyframes_ptr, h->keyframes_size, lit->keyframes);
        
        /* String table */
        const char* strtable_ptr = (const char*)&data[h->string_table_offset];
        su_insert_char(lit->string_table, 0, strtable_ptr, h->string_table_size);
    }
    
    return lit;
}

FU_FILE* lit_anim_export_to_fu(LIT_ANIM_FILE* lit)
{
    LIT_ANIM_HEADER* h = &lit->header;
    LIT_ANIM_METADATA* m = &lit->metadata;
    
    lit_anim_update(lit);
    
    FU_FILE* data_fu = fu_alloc_file();
    fu_create_mem_file(data_fu);
    fu_change_buf_size(data_fu, LIT_ANIM_HEADER_SIZE
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
    fu_write_u32(data_fu, m->anim_count, FU_BIG_ENDIAN);
    
    /* Anim offsets */
    for(uint32_t i = 0; i != cvec_size(m->anim_offsets); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        fu_write_u32(data_fu, offset, FU_BIG_ENDIAN);
    }
    
    /* Anim entries */
    for(uint32_t i = 0; i != cvec_size(lit->entries); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);
        fu_seek(data_fu, offset, FU_SEEK_SET);
        fu_write_u32(data_fu, entry->name_offset, FU_BIG_ENDIAN);
        fu_write_u8(data_fu, entry->light_type);
        fu_write_u8(data_fu, entry->attribute);
        fu_write_u8(data_fu, entry->pad1);
        fu_write_u8(data_fu, entry->pad2);
        fu_write_f32(data_fu, entry->frame_rate, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->start_frame, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->end_frame, FU_BIG_ENDIAN);
        fu_write_u32(data_fu, entry->keyframe_set_count, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_00, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_01, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_02, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_03, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->color_red, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->color_green, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->color_blue, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_07, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_08, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_09, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_0A, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_0B, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_0C, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_0D, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_0E, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->intensity, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_10, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_11, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_12, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_13, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_14, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_15, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_16, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_17, FU_BIG_ENDIAN);
        
        fu_write_f32(data_fu, entry->unk_18, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_19, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_1A, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->unk_1B, FU_BIG_ENDIAN);
        
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
    
    for(uint32_t i = 0; i != cvec_size(lit->keyframes); ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(lit->keyframes, i);
        fu_write_f32(data_fu, kf->index, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, kf->value, FU_BIG_ENDIAN);
    }
    
    /* Writing string table */
    fu_seek(data_fu, h->string_table_offset, FU_SEEK_SET);
    fu_write_data(data_fu, (const uint8_t*)lit->string_table->ptr,
                  h->string_table_size);
    
    return data_fu;
}

void lit_anim_update(LIT_ANIM_FILE* lit)
{
    LIT_ANIM_HEADER* h = &lit->header;
    LIT_ANIM_METADATA* m = &lit->metadata;
    CVEC entry_sizes = cvec_create(sizeof(uint32_t));

    /* Metadata */
    m->anim_count = cvec_size(lit->entries);

    h->metadata_offset = 0x18; /* Constant */
    h->metadata_size = 4; /* Fixed metadata fields */
    h->metadata_size += m->anim_count*4; /* Anim offsets, calculated later */

    for(uint32_t i = 0; i != cvec_size(lit->entries); ++i)
    {
        LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);
        uint32_t entry_size = 34*4 + entry->keyframe_set_count*MIRAGE_KEYFRAME_SET_SIZE;
        h->metadata_size += entry_size;
        cvec_push_back(entry_sizes, &entry_size);
    }

    /* Anim offsets */
    cvec_resize(m->anim_offsets, m->anim_count);

    /* anim offsets array starts at const offset */
    uint32_t anim_it = 0x1C + m->anim_count*4;

    for(uint32_t i = 0; i != m->anim_count; ++i)
    {
        uint32_t* offset = (uint32_t*)cvec_at(m->anim_offsets, i);
        const uint32_t cur_entry_size = *(uint32_t*)cvec_at(entry_sizes, i);
        *offset = anim_it;
        anim_it += cur_entry_size;
    }

    /* Keyframes */
    h->keyframes_offset = h->metadata_offset + h->metadata_size;
    h->keyframes_size = cvec_size(lit->keyframes) * MIRAGE_KEYFRAME_SIZE;

    /* String table */
    mirage_pad_str_table(lit->string_table, 4);
    h->string_table_offset = h->keyframes_offset + h->keyframes_size;
    h->string_table_size = lit->string_table->size;
    
    /* Cleanup */
    entry_sizes = cvec_destroy(entry_sizes);
}

LIT_ANIM_FILE* lit_anim_free(LIT_ANIM_FILE* lit)
{
    if(lit)
    {
        for(uint32_t i = 0; i != cvec_size(lit->entries); ++i)
        {
            LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);
            entry->keyframe_sets = cvec_destroy(entry->keyframe_sets);
        }
        
        lit->metadata.anim_offsets = cvec_destroy(lit->metadata.anim_offsets);
        lit->entries = cvec_destroy(lit->entries);
        lit->keyframes = cvec_destroy(lit->keyframes);
        lit->string_table = su_free(lit->string_table);
        free(lit);
    }
    
    return NULL;
}

LIT_ANIM_ENTRY* lit_anim_get_entry_by_id(CVEC entries, const uint32_t id)
{
    return (LIT_ANIM_ENTRY*)cvec_at(entries, id);
}

CVEC lit_anim_calc_offsets(LIT_ANIM_FILE* lit)
{
    CVEC offsets = cvec_create(sizeof(uint32_t));
    
    /* First six values are always there: 0, 4, 8, 12, 16, 20 */
    const uint32_t anims_offsets_ptr = lit->header.metadata_offset + 4;
    
    for(uint32_t i = 0; i != 6; ++i)
    {
        uint32_t temp = 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    for(uint32_t i = 0; i != cvec_size(lit->entries); ++i)
    {
        uint32_t temp = anims_offsets_ptr + 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    return offsets;
}