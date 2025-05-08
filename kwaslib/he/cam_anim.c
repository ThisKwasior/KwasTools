#include "cam_anim.h"

#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

#include "mirage.h"

CAM_ANIM_FILE* cam_anim_alloc()
{
    CAM_ANIM_FILE* cam = (CAM_ANIM_FILE*)calloc(1, sizeof(CAM_ANIM_FILE));
    
    if(cam)
    {
        cam->metadata.anim_offsets = cvec_create(sizeof(uint32_t));
        cam->entries = cvec_create(sizeof(CAM_ANIM_ENTRY));
        cam->keyframes = cvec_create(sizeof(MIRAGE_KEYFRAME));
        cam->string_table = su_create_string("", 0);
    }
    
    return cam;
}

CAM_ANIM_FILE* cam_anim_load_from_data(const uint8_t* data)
{
    CAM_ANIM_FILE* cam = cam_anim_alloc();
    CAM_ANIM_HEADER* h = &cam->header;
    CAM_ANIM_METADATA* m = &cam->metadata;
    
    if(cam)
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
        cvec_resize(cam->entries, m->anim_count);
        
        for(uint32_t i = 0; i != m->anim_count; ++i)
        {
            const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
            const uint8_t* entry_ptr = &data[offset];
            CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);

            entry->name_offset = tr_read_u32be(&entry_ptr[0]);
            entry->rot_or_aim = tr_read_u8(&entry_ptr[4]);
            entry->flag2 = tr_read_u8(&entry_ptr[5]);
            entry->flag3 = tr_read_u8(&entry_ptr[6]);
            entry->flag4 = tr_read_u8(&entry_ptr[7]);
            entry->frame_rate = tr_read_f32be(&entry_ptr[8]);
            entry->start_frame = tr_read_f32be(&entry_ptr[12]);
            entry->end_frame = tr_read_f32be(&entry_ptr[16]);
            entry->keyframe_set_count = tr_read_u32be(&entry_ptr[20]);
            entry->cam_pos_x = tr_read_f32be(&entry_ptr[24]);
            entry->cam_pos_z = tr_read_f32be(&entry_ptr[28]);
            entry->cam_pos_y = tr_read_f32be(&entry_ptr[32]);
            entry->cam_rot_x = tr_read_f32be(&entry_ptr[36]);
            entry->cam_rot_z = tr_read_f32be(&entry_ptr[40]);
            entry->cam_rot_y = tr_read_f32be(&entry_ptr[44]);
            entry->aim_pos_x = tr_read_f32be(&entry_ptr[48]);
            entry->aim_pos_z = tr_read_f32be(&entry_ptr[52]);
            entry->aim_pos_y = tr_read_f32be(&entry_ptr[56]);
            entry->twist = tr_read_f32be(&entry_ptr[60]);
            entry->z_near = tr_read_f32be(&entry_ptr[64]);
            entry->z_far = tr_read_f32be(&entry_ptr[68]);
            entry->fov = tr_read_f32be(&entry_ptr[72]);
            entry->aspect_ratio = tr_read_f32be(&entry_ptr[76]);
            
            entry_ptr += 80;
            entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
            mirage_read_keyframe_sets_from_data(entry_ptr,
                                                entry->keyframe_set_count,
                                                entry->keyframe_sets);
        }
        
        /* Keyframes */
        const uint8_t* keyframes_ptr = &data[h->keyframes_offset];
        mirage_read_keyframes_from_data(keyframes_ptr, h->keyframes_size, cam->keyframes);
        
        /* String table */
        const char* strtable_ptr = (const char*)&data[h->string_table_offset];
        su_insert_char(cam->string_table, 0, strtable_ptr, h->string_table_size);
    }
    
    return cam;
}

FU_FILE* cam_anim_export_to_fu(CAM_ANIM_FILE* cam)
{
    CAM_ANIM_HEADER* h = &cam->header;
    CAM_ANIM_METADATA* m = &cam->metadata;
    
    cam_anim_update(cam);
    
    FU_FILE* data_fu = fu_alloc_file();
    fu_create_mem_file(data_fu);
    fu_change_buf_size(data_fu, CAM_ANIM_HEADER_SIZE
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
    for(uint32_t i = 0; i != cvec_size(cam->entries); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(m->anim_offsets, i);
        CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);
        fu_seek(data_fu, offset, FU_SEEK_SET);
        fu_write_u32(data_fu, entry->name_offset, FU_BIG_ENDIAN);
        fu_write_u8(data_fu, entry->rot_or_aim);
        fu_write_u8(data_fu, entry->flag2);
        fu_write_u8(data_fu, entry->flag3);
        fu_write_u8(data_fu, entry->flag4);
        fu_write_f32(data_fu, entry->frame_rate, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->start_frame, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->end_frame, FU_BIG_ENDIAN);
        fu_write_u32(data_fu, entry->keyframe_set_count, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_pos_x, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_pos_z, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_pos_y, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_rot_x, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_rot_z, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->cam_rot_y, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->aim_pos_x, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->aim_pos_z, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->aim_pos_y, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->twist, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->z_near, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->z_far, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->fov, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, entry->aspect_ratio, FU_BIG_ENDIAN);
        
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
    
    for(uint32_t i = 0; i != cvec_size(cam->keyframes); ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(cam->keyframes, i);
        fu_write_f32(data_fu, kf->index, FU_BIG_ENDIAN);
        fu_write_f32(data_fu, kf->value, FU_BIG_ENDIAN);
    }
    
    /* Writing string table */
    fu_seek(data_fu, h->string_table_offset, FU_SEEK_SET);
    fu_write_data(data_fu, (const uint8_t*)cam->string_table->ptr,
                  h->string_table_size);
    
    return data_fu;
}

void cam_anim_update(CAM_ANIM_FILE* cam)
{
    CAM_ANIM_HEADER* h = &cam->header;
    CAM_ANIM_METADATA* m = &cam->metadata;
    CVEC entry_sizes = cvec_create(sizeof(uint32_t));
    
    /* Metadata */
    m->anim_count = cvec_size(cam->entries);

    h->metadata_offset = 0x18; /* Constant */
    h->metadata_size = 4; /* Fixed metadata fields */
    h->metadata_size += m->anim_count*4; /* Anim offsets, calculated later */
    
    for(uint32_t i = 0; i != cvec_size(cam->entries); ++i)
    {
        CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);
        uint32_t entry_size = 20*4 + entry->keyframe_set_count*MIRAGE_KEYFRAME_SET_SIZE;
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
    h->keyframes_size = cvec_size(cam->keyframes) * MIRAGE_KEYFRAME_SIZE;

    /* String table */
    mirage_pad_str_table(cam->string_table, 4);
    h->string_table_offset = h->keyframes_offset + h->keyframes_size;
    h->string_table_size = cam->string_table->size;
    
    /* Cleanup */
    entry_sizes = cvec_destroy(entry_sizes);
}

CAM_ANIM_FILE* cam_anim_free(CAM_ANIM_FILE* cam)
{
    if(cam)
    {
        for(uint32_t i = 0; i != cvec_size(cam->entries); ++i)
        {
            CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);
            entry->keyframe_sets = cvec_destroy(entry->keyframe_sets);
        }
        
        cam->metadata.anim_offsets = cvec_destroy(cam->metadata.anim_offsets);
        cam->entries = cvec_destroy(cam->entries);
        cam->keyframes = cvec_destroy(cam->keyframes);
        cam->string_table = su_free(cam->string_table);
        free(cam);
    }
    
    return NULL;
}

CAM_ANIM_ENTRY* cam_anim_get_entry_by_id(CVEC entries, const uint32_t id)
{
    return (CAM_ANIM_ENTRY*)cvec_at(entries, id);
}

CVEC cam_anim_calc_offsets(CAM_ANIM_FILE* cam)
{
    CVEC offsets = cvec_create(sizeof(uint32_t));
    
    /* First six values are always there: 0, 4, 8, 12, 16, 20 */
    const uint32_t offset_count = 6 + cvec_size(cam->entries);
    
    /*
        In cam-anim, metadata only has the count and offsets.
        For some reason, anim count is being treated as an offset,
        and the last actual offset is left out of the offset table.
    */
    for(uint32_t i = 0; i != offset_count; ++i)
    {
        uint32_t temp = 4*i;
        cvec_push_back(offsets, &temp);
    }
    
    return offsets;
}