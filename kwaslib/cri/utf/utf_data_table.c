#include "utf_data_table.h"

#include <string.h>

#include <kwaslib/core/math/boundary.h>

#include "utf_defines.h"
#include "utf_string_table.h"

const uint32_t utf_add_data_to_table(SU_STRING* data_table,
                                     const uint8_t* data,
                                     const uint32_t size)
{
    if(size == 0) return 0;
    
    const uint8_t is_md5 = (size == 16) ? 1 : 0;
    uint8_t is_utf = 0;
    
    if(size > 4)
        if(su_cmp_char((const char*)data, 4, UTF_MAGIC, 4) == 0)
            is_utf = 1;
    
    if(is_utf || is_md5)
    {
        utf_pad_data_table(data_table, UTF_DATA_BLOCK_SIZE);
    }
    
    const uint32_t offset = data_table->size;
    su_insert_char(data_table, -1, (const char*)data, size);

    return offset;
}

const uint8_t* utf_get_ptr_in_data_table(SU_STRING* data_table,
                                         const uint32_t offset)
{
    return (const uint8_t*)&data_table->ptr[offset];
}

void utf_pad_data_table(SU_STRING* data_table, const uint32_t block_size)
{
    utf_pad_str_table(data_table, block_size);
}