#include "utf_data_table.h"

#include <string.h>

#include <kwaslib/core/math/boundary.h>

#include "utf_defines.h"
#include "utf_string_table.h"

const uint32_t utf_add_data_to_table(SU_STRING* data_table,
                                     const uint8_t* data,
                                     const uint32_t size,
                                     const uint8_t insert_pad)
{
    if(size == 0)
    {
        return 0;
    }
    
    const uint32_t offset = data_table->size;
    su_insert_char(data_table, -1, (const char*)data, size);
    
    if(insert_pad)
    {
        utf_pad_data_table(data_table, UTF_DATA_BLOCK_SIZE);
    }
    
    return offset;


    /*if(size == 0)
    {
        return 0;
    }

    uint8_t fix = 0;

    if(size == 16)
    {
        //fix = 1;
    }
    else if(size > 16)
    {
        if(su_cmp_char((const char*)data, 4, UTF_MAGIC, 4) == 0) fix = 1;
        else if(su_cmp_char((const char*)data, 4, "AFS2", 4) == 0) fix = 1;
        //fix=1;
    }
    
    // Errors object sounds in SXSG
    if(fix) utf_pad_data_table(data_table, UTF_DATA_BLOCK_SIZE);
    
    const uint32_t offset = data_table->size;
    su_insert_char(data_table, -1, (const char*)data, size);

    return offset;
    */
}

const uint8_t* utf_get_ptr_in_data_table(SU_STRING* data_table,
                                         const uint32_t offset)
{
    return (const uint8_t*)&data_table->ptr[offset];
}

void utf_pad_data_table(SU_STRING* data_table, const uint32_t block_size)
{
    utf_pad_str_table(data_table, block_size);
    return;
    
    //const uint32_t pad = bound_calc_leftover(block_size, data_table->size);
    //if(pad == 0) utf_add_zeros_to_str(data_table, block_size);
    //else utf_add_zeros_to_str(data_table, pad);
}