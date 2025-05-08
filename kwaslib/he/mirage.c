#include "mirage.h"

#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/math/boundary.h>

MIRAGE_FILE* mirage_alloc()
{
    MIRAGE_FILE* mirage = (MIRAGE_FILE*)calloc(1, sizeof(MIRAGE_FILE));
    
    if(mirage)
    {
        mirage->offset_table = cvec_create(sizeof(uint32_t));
        mirage->file_name = su_create_string("", 0);
    }
    
    return mirage;
}

MIRAGE_FILE* mirage_load_from_data(const uint8_t* data)
{
    MIRAGE_FILE* mirage = mirage_alloc();
    
    if(mirage)
    {
        MIRAGE_HEADER* h = &mirage->header;
        
        /* Header */
        h->file_size = tr_read_u32be(&data[0]);
        h->data_version = tr_read_u32be(&data[4]);
        h->data_size = tr_read_u32be(&data[8]);
        h->data_offset = tr_read_u32be(&data[12]);
        h->offset_table_offset = tr_read_u32be(&data[16]);
        h->file_name_offset = tr_read_u32be(&data[20]);
        
        /* Data section */
        mirage->data = tr_read_array_alloc(&data[h->data_offset], h->data_size);

        /* Offsets */
        const uint8_t* offsets_ptr = &data[h->offset_table_offset];
        const uint32_t offsets_count = tr_read_u32be(offsets_ptr);
        offsets_ptr += 4;
        
        if(offsets_count)
        {
            cvec_resize(mirage->offset_table, offsets_count);
            
            for(uint32_t i = 0; i != offsets_count; ++i)
            {
                uint32_t* cur_offset = (uint32_t*)cvec_at(mirage->offset_table, i);
                *cur_offset = tr_read_u32be(offsets_ptr);
                offsets_ptr += 4;
            }
        }
        
        /* File name */
        if(h->file_name_offset)
        {
            const char* name_ptr = (const char*)&data[h->file_name_offset];
            const uint32_t name_len = strlen(name_ptr);
            su_insert_char(mirage->file_name, 0, name_ptr, name_len);
        }
    }
    
    return mirage;
}

FU_FILE* mirage_export_to_fu(MIRAGE_FILE* mirage)
{
    if(mirage == NULL)
    {
        return NULL;
    }
    
    mirage_update(mirage);

    FU_FILE* miragef = fu_alloc_file();
    fu_create_mem_file(miragef);
    fu_change_buf_size(miragef, mirage->header.file_size);
    
    /* Writing header */
    fu_seek(miragef, 0, FU_SEEK_SET);
    fu_write_u32(miragef, mirage->header.file_size, FU_BIG_ENDIAN);
    fu_write_u32(miragef, mirage->header.data_version, FU_BIG_ENDIAN);
    fu_write_u32(miragef, mirage->header.data_size, FU_BIG_ENDIAN);
    fu_write_u32(miragef, mirage->header.data_offset, FU_BIG_ENDIAN);
    fu_write_u32(miragef, mirage->header.offset_table_offset, FU_BIG_ENDIAN);
    fu_write_u32(miragef, mirage->header.file_name_offset, FU_BIG_ENDIAN);
    
    /* Data */
    fu_seek(miragef, mirage->header.data_offset, FU_SEEK_SET);
    fu_write_data(miragef, mirage->data, mirage->header.data_size);
    
    /* Offset table */
    fu_seek(miragef, mirage->header.offset_table_offset, FU_SEEK_SET);
    fu_write_u32(miragef, cvec_size(mirage->offset_table), FU_BIG_ENDIAN);
    
    for(uint32_t i = 0; i != cvec_size(mirage->offset_table); ++i)
    {
        const uint32_t offset = *(uint32_t*)cvec_at(mirage->offset_table, i);
        fu_write_u32(miragef, offset, FU_BIG_ENDIAN);
    }
    
    /* File name */
    if(mirage->header.file_name_offset)
    {
        fu_seek(miragef, mirage->header.file_name_offset, FU_SEEK_SET);
        fu_write_data(miragef,
                      (const uint8_t*)mirage->file_name->ptr,
                      mirage->file_name->size);
    }
    
    return miragef;
}

void mirage_update(MIRAGE_FILE* mirage)
{
    MIRAGE_HEADER* h = &mirage->header;
    
    uint32_t file_size = MIRAGE_HEADER_SIZE;
    
    h->data_offset = file_size;
    file_size += h->data_size;
    file_size += bound_calc_leftover(4, file_size);
    
    h->offset_table_offset = file_size;
    file_size += 4;
    file_size += 4*cvec_size(mirage->offset_table);
    
    h->file_name_offset = 0;
    if(mirage->file_name->size)
    {
        h->file_name_offset = file_size;
        file_size += mirage->file_name->size + 1; /* Gotta include terminator */
        file_size += bound_calc_leftover(4, file_size);
    }
    
    h->file_size = file_size;
}

void mirage_set_data(MIRAGE_FILE* mirage, const uint8_t* data, const uint32_t size)
{
    free(mirage->data);
    mirage->header.data_size = size;
    mirage->data = (uint8_t*)calloc(1, mirage->header.data_size);
    memcpy(mirage->data, data, mirage->header.data_size);
}

MIRAGE_FILE* mirage_free(MIRAGE_FILE* mirage)
{
    if(mirage)
    {
        free(mirage->data);
        mirage->offset_table = cvec_destroy(mirage->offset_table);
        mirage->file_name = su_free(mirage->file_name);
        free(mirage);
    }
    
    return NULL;
}