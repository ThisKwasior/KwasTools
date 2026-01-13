#include "afs_export.h"

#include <kwaslib/core/math/boundary.h>

const uint32_t afs_get_first_entry_id(AFS_FILE* afs)
{
    for(uint32_t i = 0; i != AFS_MAX_FILES; ++i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->data && entry->size)
        {
            return i;
        }
    }
    
    return (uint32_t)AFS_ERROR;
}

const uint32_t afs_get_last_entry_id(AFS_FILE* afs)
{
    for(int32_t i = (AFS_MAX_FILES-1); i != -1; --i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->data && entry->size)
        {
            return i;
        }
    }
    
    return (uint32_t)AFS_ERROR;
}

const uint32_t afs_get_data_section_size(AFS_FILE* afs, const uint32_t block_size)
{
    uint32_t size = 0;
    
    for(uint32_t i = 0; i != AFS_MAX_FILES; ++i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->data && entry->size)
        {
            size += entry->size;
            const uint32_t left = bound_calc_leftover(block_size, size);
            size += left;
        }
    }
    
    return size;
}