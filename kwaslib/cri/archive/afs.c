#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/date_utils.h>
#include <kwaslib/core/math/boundary.h>
#include <kwaslib/cri/audio/adx.h>

#include "afs.h"
#include "afs_parse.h"
#include "afs_export.h"

AFS_FILE* afs_alloc()
{
    AFS_FILE* afs = (AFS_FILE*)calloc(1, sizeof(AFS_FILE));
    afs->entries = cvec_create(sizeof(AFS_ENTRY));
    cvec_resize(afs->entries, AFS_MAX_FILES);
    afs->has_metadata = AFS_NO_METADATA;
    return afs;
}

AFS_FILE* afs_read_from_fu(FU_FILE* fafs)
{
    return afs_read_from_data((uint8_t*)fafs->buf, fafs->size);
}

AFS_FILE* afs_read_from_data(const uint8_t* data, const uint32_t size)
{
    if(afs_check_if_valid(data, size) == AFS_ERROR)
    {
        return NULL;
    }

    /* Backbone of correct AFS handling */
    const uint32_t first_file_id = afs_find_first_file_index(data, size);
    
    if(first_file_id == AFS_ERROR)
    {
        return NULL;
    }
    
    uint32_t file_count = afs_count_possible_files(first_file_id, data);
    const uint32_t metadata_index = afs_find_metadata_index(first_file_id, file_count, data);
    const uint32_t metadata_offset = afs_id_to_entry_offset(metadata_index);
    const uint32_t metadata_pos = tr_read_u32le(&data[metadata_offset]);
    const uint32_t metadata_size = tr_read_u32le(&data[metadata_offset+4]);
    
    AFS_FILE* afs = afs_alloc();
    
    if(metadata_pos && metadata_size)
    {
        afs->has_metadata = AFS_HAS_METADATA;
        file_count -= 1;
    }
    
    /* Read entries */
    uint32_t metadata_it = 0;
    
    for(uint32_t i = first_file_id; i != metadata_index; ++i)
    {
        const uint32_t entry_pos = afs_id_to_entry_offset(i);
        const uint32_t entry_offset = tr_read_u32le(&data[entry_pos]);
        const uint32_t entry_size = tr_read_u32le(&data[entry_pos+4]);
        
        if(entry_offset && entry_size)
        {
            AFS_ENTRY* cur_entry = afs_get_entry_by_id(afs, i);
                        
            cur_entry->data = (uint8_t*)calloc(1, entry_size);
            cur_entry->size = entry_size;
            cur_entry->data_type = afs_get_file_type(&data[entry_offset], entry_size);
            memcpy(&cur_entry->data[0], &data[entry_offset], entry_size);
            
            if(afs->has_metadata)
            {
                const uint32_t meta_offset = afs_id_to_metadata_offset(metadata_pos, metadata_size, metadata_it);
                
                tr_read_array(&data[meta_offset], AFS_ENTRY_METADATA_NAME_SIZE, (uint8_t*)cur_entry->metadata.name);
                cur_entry->metadata.year = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE]);
                cur_entry->metadata.month = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+2]);
                cur_entry->metadata.day = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+4]);
                cur_entry->metadata.hour = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+6]);
                cur_entry->metadata.minute = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+8]);
                cur_entry->metadata.second = tr_read_u16le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+10]);
                cur_entry->metadata.file_size = tr_read_u32le(&data[meta_offset+AFS_ENTRY_METADATA_NAME_SIZE+12]);
                
                metadata_it += 1;
            }
        }
    }
    
    return afs;
}

FU_FILE* afs_write_to_fu(AFS_FILE* afs, const uint32_t block_size)
{
    const uint32_t file_count = afs_get_count(afs);
    const uint32_t first_file_id = afs_get_first_entry_id(afs);
    const uint32_t last_file_id = afs_get_last_entry_id(afs);
    
    const uint32_t metadata_entry_pos = afs_id_to_entry_offset(last_file_id+1);
    const uint32_t metadata_size = file_count*AFS_ENTRY_METADATA_SIZE;
    const uint32_t metadata_size_aligned = metadata_size + bound_calc_leftover(block_size, metadata_size);
    
    uint32_t data_offset = metadata_entry_pos+AFS_ENTRY_SIZE;
    data_offset += bound_calc_leftover(block_size, data_offset);
    const uint32_t data_size = afs_get_data_section_size(afs, block_size);
    
    const uint32_t metadata_offset = data_offset + data_size;
    const uint32_t file_size = metadata_offset + metadata_size_aligned;
    
    FU_FILE* fafs = fu_alloc_file();
    fu_create_mem_file(fafs);
    fu_change_buf_size(fafs, file_size);
    fu_seek(fafs, 0, FU_SEEK_SET);
    
    fu_write_data(fafs, (uint8_t*)AFS_MAGIC, 4);
    fu_write_u32(fafs, file_count, FU_LITTLE_ENDIAN);
    
    fu_seek(fafs, metadata_entry_pos, FU_SEEK_SET);
    fu_write_u32(fafs, metadata_offset, FU_LITTLE_ENDIAN);
    fu_write_u32(fafs, metadata_size, FU_LITTLE_ENDIAN);
    
    uint32_t data_counter = data_offset;
    uint32_t metadata_counter = metadata_offset;
    for(uint32_t i = first_file_id; i != (last_file_id+1); ++i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->data && entry->size)
        {
            const uint32_t entry_offset = afs_id_to_entry_offset(i);
            fu_seek(fafs, entry_offset, FU_SEEK_SET);
            
            fu_write_u32(fafs, data_counter, FU_LITTLE_ENDIAN);
            fu_write_u32(fafs, entry->size, FU_LITTLE_ENDIAN);
            
            fu_seek(fafs, data_counter, FU_SEEK_SET);
            fu_write_data(fafs, entry->data, entry->size);
            
            data_counter += entry->size;
            data_counter += bound_calc_leftover(block_size, data_counter);
            
            if(afs->has_metadata)
            {
                fu_seek(fafs, metadata_counter, FU_SEEK_SET);
                
                fu_write_data(fafs, (uint8_t*)entry->metadata.name, AFS_ENTRY_METADATA_NAME_SIZE);
                fu_write_u16(fafs, entry->metadata.year, FU_LITTLE_ENDIAN);
                fu_write_u16(fafs, entry->metadata.month, FU_LITTLE_ENDIAN);
                fu_write_u16(fafs, entry->metadata.day, FU_LITTLE_ENDIAN);
                fu_write_u16(fafs, entry->metadata.hour, FU_LITTLE_ENDIAN);
                fu_write_u16(fafs, entry->metadata.minute, FU_LITTLE_ENDIAN);
                fu_write_u16(fafs, entry->metadata.second, FU_LITTLE_ENDIAN);
                fu_write_u32(fafs, entry->size, FU_LITTLE_ENDIAN);
                
                metadata_counter += AFS_ENTRY_METADATA_SIZE;
            }
        }
    }
        
    return fafs;
}

AFS_FILE* afs_free(AFS_FILE* afs)
{
    for(uint64_t i = 0; i != AFS_MAX_FILES; ++i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->size)
        {
            free(entry->data);
        }
    }
    
    afs->entries = cvec_destroy(afs->entries);
    free(afs);
    
    return NULL;
}

AFS_ENTRY* afs_get_entry_by_id(AFS_FILE* afs, const uint64_t id)
{
    return (AFS_ENTRY*)cvec_at(afs->entries, id);
}

AFS_ENTRY* afs_set_entry_data(AFS_FILE* afs, const uint32_t id,
                              const uint8_t* data, const uint32_t size,
                              const char* name, const time_t timestamp)
{
    AFS_ENTRY* entry = afs_get_entry_by_id(afs, id);
    
    /* Entry is already present. Remove it */
    if(entry->data)
    {
        afs_remove_entry(afs, id);
    }

    entry->data_type = afs_get_file_type(data, size);
    entry->size = size;
    entry->data = (uint8_t*)calloc(1, size);
    memcpy(entry->data, data, size);
    
    entry->metadata.file_size = size;
    
    int y,m,d,h,min,s;
    du_epoch_to_values(timestamp, &y, &m, &d, &h, &min, &s);
    entry->metadata.year = y;
    entry->metadata.month = m;
    entry->metadata.day = d;
    entry->metadata.hour = h;
    entry->metadata.minute = min;
    entry->metadata.second = s;

    if(name)
    {
        uint32_t name_len = strlen(name);
        
        if(name_len != 0)
        {
            /* Name + null*/
            const uint32_t max_name = AFS_ENTRY_METADATA_NAME_SIZE - 1;
            
            if(name_len > max_name)
            {
                name_len = max_name;
            }
            
            sprintf(entry->metadata.name, "%.*s", name_len, name);
        }
        else
        {
            goto insert_name_id;
        }
    }
    else
    {
insert_name_id:
        sprintf(entry->metadata.name, "%05d.bin", id);
    }

    return entry;
}

void afs_remove_entry(AFS_FILE* afs, const uint32_t id)
{
    AFS_ENTRY* entry = afs_get_entry_by_id(afs, id);
    
    if(entry->data)
    {
        free(entry->data);
        memset(entry, 0, sizeof(AFS_ENTRY));
    }
}

const uint32_t afs_get_count(AFS_FILE* afs)
{
    uint32_t count = 0;
    
    if(afs)
    {
        for(uint32_t i = 0; i != AFS_MAX_FILES; ++i)
        {
            AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
            
            if(entry->data && entry->size)
            {
                count += 1;
            }
        }
    }
    
    return count;
}

const uint32_t afs_id_to_entry_offset(const uint32_t id)
{
    return AFS_HEADER_SIZE+(id*AFS_ENTRY_SIZE);
}

const uint32_t afs_id_to_metadata_offset(const uint32_t meta_pos, const uint32_t size, const uint32_t id)
{
    if(id >= AFS_MAX_FILES)
    {
        return AFS_ERROR;
    }
    
    const uint32_t pos = (AFS_ENTRY_METADATA_SIZE*id);
    
    if(pos >= size)
    {
        return AFS_ERROR;
    }
    
    return meta_pos+pos;
}

const uint8_t afs_get_file_type(const uint8_t* data, const uint32_t size)
{
    const uint8_t is_adx = adx_check_if_valid(data, size);
    const uint8_t is_afs = (afs_check_if_valid(data, size) == AFS_GOOD);
    
    if(is_adx == ADX_TYPE_ADX) return AFS_DATA_ADX;
    if(is_adx == ADX_TYPE_AHX) return AFS_DATA_AHX;
    if(is_afs) return AFS_DATA_AFS;
    
    return AFS_DATA_BIN;
}
