#pragma once

#include <kwaslib/core/io/string_utils.h>

/*
    Appends data to the table.
    
    Returns the offset of the data.
*/
const uint32_t utf_add_data_to_table(SU_STRING* data_table,
                                     const uint8_t* data,
                                     const uint32_t size,
                                     const uint8_t insert_pad);

const uint8_t* utf_get_ptr_in_data_table(SU_STRING* data_table,
                                         const uint32_t offset);

/*
    Pads the data table with zeros to align to block size.
*/
void utf_pad_data_table(SU_STRING* data_table, const uint32_t block_size);