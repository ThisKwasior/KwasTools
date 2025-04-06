#include "dat.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <kwaslib/core/crypto/crc32.h>
#include <kwaslib/core/math/boundary.h>

/*
	Implementation
*/

DAT_FILE* dat_parse_file(FU_FILE* file)
{
    DAT_FILE* dat = dat_alloc_dat();
    DAT_HEADER* header = &dat->header;
    
    if(dat == NULL)
    {
        return NULL;
    }
    
    /* Reading the magic and checking it */
    fu_read_data(file, &header->magic[0], 4, NULL);
    
    if(su_cmp_char((const char*)&header->magic[0], 4, DAT_MAGIC, 4) != 0)
    {
        /* Data is incorrect, aborting */
        free(dat);
        return NULL;
    }
    
    header->file_count = fu_read_u32(file, NULL, FU_LITTLE_ENDIAN);
    
    const uint8_t endian = dat_check_endian(header->file_count);
    printf("DAT file is in %s Endian\n", endian == 1 ? "Little" : "Big");
    
    /* Going back and reading file count again with correct endianness */
    fu_seek(file, -4, SEEK_CUR);
    header->file_count = fu_read_u32(file, NULL, endian);
    
    /* Reading all other fields */
    header->positions_offset = fu_read_u32(file, NULL, endian);
    header->extensions_offset = fu_read_u32(file, NULL, endian);
    header->names_offset = fu_read_u32(file, NULL, endian);
    header->sizes_offset = fu_read_u32(file, NULL, endian);
    header->hashtable_offset = fu_read_u32(file, NULL, endian);
    header->unk1C = fu_read_u32(file, NULL, endian);
    
    /* Populating entries */
    cvec_resize(dat->entries, header->file_count);
    
    fu_seek(file, header->names_offset, SEEK_SET);
    dat->entry_name_size = fu_read_u32(file, NULL, endian);
    char* name_temp = (char*)calloc(1, dat->entry_name_size+1);
    
    for(uint32_t i = 0; i != header->file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        const uint64_t pos = header->positions_offset+4*i;
        const uint64_t ext = header->extensions_offset+4*i;
        const uint64_t name = (header->names_offset+4)+(dat->entry_name_size*i);
        const uint64_t size = header->sizes_offset+4*i;
        
        fu_seek(file, pos, SEEK_SET);
        entry->position = fu_read_u32(file, NULL, endian);
        
        fu_seek(file, ext, SEEK_SET);
        fu_read_data(file, &entry->extension[0], 4, NULL);

        fu_seek(file, name, SEEK_SET);
        fu_read_data(file, (uint8_t*)name_temp, dat->entry_name_size, NULL);
        entry->name = su_create_string(name_temp, strlen(name_temp));
        memset(name_temp, 0, dat->entry_name_size+1);
        
        fu_seek(file, size, SEEK_SET);
        entry->size = fu_read_u32(file, NULL, endian);
        
        fu_seek(file, entry->position, SEEK_SET);
        entry->data = (uint8_t*)calloc(1, entry->size);
        fu_read_data(file, &entry->data[0], entry->size, NULL);
    }
    
    free(name_temp);
    
    /* Reading the hashtable */
    fu_seek(file, header->hashtable_offset, SEEK_SET);
    dat->hashtable = dat_hash_read_table(file, header->file_count, endian);
    
    return dat;
}

FU_FILE* dat_to_fu_file(DAT_FILE* dat, const uint32_t block_size, const uint8_t endian)
{
    FU_FILE* file = fu_alloc_file();
    fu_create_mem_file(file);
    
    /* Writing header */
    fu_write_data(file, &dat->header.magic[0], 4);
    fu_write_u32(file, dat->header.file_count, endian);
    fu_write_u32(file, dat->header.positions_offset, endian);
    fu_write_u32(file, dat->header.extensions_offset, endian);
    fu_write_u32(file, dat->header.names_offset, endian);
    fu_write_u32(file, dat->header.sizes_offset, endian);
    fu_write_u32(file, dat->header.hashtable_offset, endian);
    fu_write_u32(file, dat->header.unk1C, endian);
    
    fu_change_buf_size(file, dat->header.hashtable_offset);
    
	/* Write positions */
	fu_seek(file, dat->header.positions_offset, FU_SEEK_SET);
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        fu_write_u32(file, entry->position, endian);
        fu_change_buf_size(file, entry->position);
    }
    
	/* Write extensions */
	fu_seek(file, dat->header.extensions_offset, FU_SEEK_SET);
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        fu_write_data(file, &entry->extension[0], 4);
    }
    
	/* Write names */
	fu_seek(file, dat->header.names_offset, FU_SEEK_SET);
    fu_write_u32(file, dat->entry_name_size, endian);
    char* name_temp = (char*)calloc(1, dat->entry_name_size);
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        memcpy(name_temp, entry->name->ptr, entry->name->size);
        fu_write_data(file, (uint8_t*)name_temp, dat->entry_name_size);
        memset(name_temp, 0, dat->entry_name_size);
    }
    free(name_temp);
    
 	/* Write sizes */
	fu_seek(file, dat->header.sizes_offset, FU_SEEK_SET);
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        fu_write_u32(file, entry->size, endian);
    }
    
    /* Write hashtable */
    fu_seek(file, dat->header.hashtable_offset, FU_SEEK_SET);
    FU_FILE* fht = dat_hash_to_fu_file(dat->hashtable, endian);
    fu_write_data(file, (uint8_t*)fht->buf, fht->size);
    fu_close(fht);
    
    /* Write file data */
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        fu_seek(file, entry->position, FU_SEEK_SET);
        fu_write_data(file, entry->data, entry->size);
    }
    
    fu_add_to_buf_size(file, bound_calc_leftover(block_size, file->size));
    
    return file;
}

void dat_update(DAT_FILE* dat, const uint32_t block_size)
{
    /* Header */
    memcpy(dat->header.magic, DAT_MAGIC, 4);
    dat->header.file_count = cvec_size(dat->entries);
    
    dat->entry_name_size = dat_max_name_len(dat->entries);
    dat->hashtable = dat_hash_create_table(dat);
    const uint64_t hashtable_size = 4*4
                                    + (cvec_size(dat->hashtable->bucket)*2)
                                    + (cvec_size(dat->hashtable->entries)*6);
    
    /* Header - offsets */
    uint32_t cur_pos = 0x20;
    dat->header.positions_offset = cur_pos;
    
    cur_pos += 4*dat->header.file_count;
    dat->header.extensions_offset = cur_pos;

    cur_pos += 4*dat->header.file_count;
    dat->header.names_offset = cur_pos;
    
    cur_pos += 4 + dat->entry_name_size*dat->header.file_count;
    cur_pos += bound_calc_leftover(4, cur_pos); /* Sections are 4byte aligned */
    dat->header.sizes_offset = cur_pos;
    
    cur_pos += 4*dat->header.file_count;
    dat->header.hashtable_offset = cur_pos;
    
    cur_pos += hashtable_size;
    cur_pos += bound_calc_leftover(block_size, cur_pos);
    
    for(uint32_t i = 0; i != dat->header.file_count; ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        entry->position = cur_pos;
        cur_pos += entry->size;
        cur_pos += bound_calc_leftover(block_size, cur_pos);
    }
}


void dat_append_entry(CVEC entries,
                      const char* extension,
                      const char* name,
                      const uint32_t size,
                      const uint8_t* data)
{
    DAT_FILE_ENTRY entry = {0};
    
    const uint32_t ext_size = (strlen(extension) >= 3) ? 3 : strlen(extension);
    const uint32_t name_size = strlen(name);
    
    memcpy(&entry.extension[0], extension, ext_size);
    entry.name = su_create_string(name, name_size);
    entry.size = size;
    
    entry.data = (uint8_t*)calloc(1, size);
    memcpy(entry.data, data, size);
    
    cvec_push_back(entries, &entry);
}

DAT_FILE* dat_alloc_dat()
{
    DAT_FILE* dat = (DAT_FILE*)calloc(1, sizeof(DAT_FILE));
    dat->entries = cvec_create(sizeof(DAT_FILE_ENTRY));
    return dat;
}

DAT_FILE* dat_destroy(DAT_FILE* dat)
{
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        entry->name = su_free(entry->name);
        free(entry->data);
    }
    
    dat->entries = cvec_destroy(dat->entries);
    dat->hashtable = dat_hash_destroy(dat->hashtable);
    free(dat);
    return NULL;
}

DAT_FILE_ENTRY* dat_get_entry_by_id(CVEC entries, const uint64_t id)
{
    return (DAT_FILE_ENTRY*)cvec_at(entries, id);
}

uint8_t dat_check_endian(const uint32_t file_count)
{
    if(file_count >= 16777216)
    {
        return FU_BIG_ENDIAN;
    }
    
    return FU_LITTLE_ENDIAN;
}

const uint32_t dat_max_name_len(CVEC entries)
{
    uint32_t len = 0;
    
    for(uint32_t i = 0; i != cvec_size(entries); ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(entries, i);
        
        if(len < entry->name->size)
        {
            len = entry->name->size;
        }
    }
    
    return len + 1;
}