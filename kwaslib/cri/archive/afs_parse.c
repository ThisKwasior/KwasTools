#include "afs_parse.h"

#include <kwaslib/core/io/type_readers.h>

const uint8_t afs_check_if_valid(const uint8_t* data, const uint32_t size)
{
    /* File is too small to be an AFS archive */
    if(AFS_FILE_MIN_SIZE > size)
    {
        return AFS_ERROR;
    }
    
    /* Checking the header */
    uint8_t magic_stat = su_cmp_char((const char*)data, 4, AFS_MAGIC, 4);
    
    /* File is not a valid AFS archive */
    if(magic_stat != SU_STRINGS_MATCH)
    {
        return AFS_ERROR;
    }
    
    /* Useless value */
    /* const uint32_t file_count = tr_read_u32le(&data[4]); */

    return AFS_GOOD;
}

const uint32_t afs_find_first_file_index(const uint8_t* data, const uint32_t size)
{
    for(uint32_t i = 0; i != AFS_MAX_FILES; i+=1)
    {
        const uint32_t cur_offset_offset = AFS_HEADER_SIZE + (i*AFS_ENTRY_SIZE);
        
        if(cur_offset_offset >= size)
        {
            break;
        }
        
        const uint32_t cur_offset = tr_read_u32le(&data[cur_offset_offset]);
        
        if(cur_offset != 0)
        {
            return i;
        }
    }
    
    /* Can't find the first file index. File is probably corrupted or empty */
    return AFS_ERROR;
};

const uint32_t afs_count_possible_files(const uint32_t first_file_id, const uint8_t* data)
{
    uint32_t file_it = 0;
    const uint32_t fe_off = afs_id_to_entry_offset(first_file_id); 
    const uint32_t first_entry_offset = tr_read_u32le(&data[fe_off]);
    
    for(uint32_t i = first_file_id; i != (AFS_MAX_FILES+1); i+=1)
    {
        const uint32_t e_off = afs_id_to_entry_offset(i);
        
        if(e_off >= first_entry_offset)
        {
            /* Reached the first file while iterating */
            break;
        }

        const uint32_t cur_entry_offset = tr_read_u32le(&data[e_off]);

        if(cur_entry_offset)
        {
            file_it += 1;
        }
    }
    
    return file_it;
};

uint32_t afs_find_metadata_index(const uint32_t first_file_id, const uint32_t file_count, const uint8_t* data)
{
    const uint32_t fe_offset = afs_id_to_entry_offset(first_file_id);
    const uint32_t first_entry_offset = tr_read_u32le(&data[fe_offset]);
    const uint32_t feo = first_entry_offset-AFS_HEADER_SIZE;
    uint32_t max_index = ((feo-(feo%8))/8)-1;
    
    if(max_index > AFS_MAX_FILES)
    {
        max_index = AFS_MAX_FILES-1;
    }
    
    for(uint32_t i = max_index; i != first_file_id; i -= 1)
    {
        const uint32_t ce_off = afs_id_to_entry_offset(i);
        const uint32_t cur_entry_offset = tr_read_u32le(&data[ce_off]);
        const uint32_t cur_entry_size = tr_read_u32le(&data[ce_off+4]);
        
        if(cur_entry_offset)
        {
            const uint32_t meta_count = cur_entry_size/AFS_ENTRY_METADATA_SIZE;

            if(meta_count == (file_count-1))
            {
                return i;
            }
            else /* No metadata section */
            {
                return i+1;
            }
        }
    }
    
    /* No metadata section */
    return 1;
};