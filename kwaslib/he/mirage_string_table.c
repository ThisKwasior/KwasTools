#include "mirage_string_table.h"

#include <string.h>

#include <kwaslib/core/math/boundary.h>

const uint32_t mirage_add_str_to_table(SU_STRING* string_table,
                                       const char* str,
                                       const uint32_t size)
{
    uint32_t index = -1;
    
    if(string_table)
    {
        index = string_table->size;
        su_insert_char(string_table, -1, str, size);
        su_insert_char(string_table, -1, "\0", 1);
    }
    
    return index;
}

const char* mirage_get_ptr_in_table(SU_STRING* string_table,
                                    const uint32_t offset)
{
    return (const char*)&string_table->ptr[offset];
}

void mirage_pad_str_table(SU_STRING* string_table, const uint32_t block_size)
{
    const uint32_t pad = bound_calc_leftover(block_size, string_table->size);

    for(uint32_t i = 0; i != pad; ++i)
    {
        su_insert_char(string_table, -1, "\0", 1);
    }
}

const uint32_t mirage_str_table_count(SU_STRING* string_table)
{
    uint32_t tex_ptr_it = 0;
    uint32_t tex_it = 0;
    while(tex_ptr_it != string_table->size)
    {
        const char* cur_str_ptr = &string_table->ptr[tex_ptr_it];
        const uint32_t cur_str_len = strlen(cur_str_ptr);
        
        if(cur_str_len)
        {
            tex_it += 1;
        }
        
        tex_ptr_it += cur_str_len + 1;
    }
    
    return tex_it;
}