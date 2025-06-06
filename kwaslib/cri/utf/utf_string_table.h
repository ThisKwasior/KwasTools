#pragma once

#include <kwaslib/core/io/string_utils.h>

/*
    Appends a string+NULL to the table.
    
    Returns the index of the string.
*/
const uint32_t utf_add_str_to_table(SU_STRING* string_table,
                                    const char* str,
                                    const uint32_t size);
                                       
                                       
/*
    Returns a pointer to a string in string table.
*/
const char* utf_get_ptr_in_table(SU_STRING* string_table,
                                 const uint32_t offset);
                                    
/*
    Pads the string table with zeros to align to block size.
*/
void utf_pad_str_table(SU_STRING* string_table, const uint32_t block_size);

void utf_add_zeros_to_str(SU_STRING* string_table, const uint32_t pad);

/*
    Returns amount of strings in the table
*/
const uint32_t utf_str_table_count(SU_STRING* string_table);