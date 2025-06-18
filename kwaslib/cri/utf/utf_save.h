#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/cvector.h>

#include "utf_defines.h"
#include "utf_table.h"

FU_FILE* utf_save_to_fu(UTF_TABLE* utf);

/*
    Returns a cvector of UTF_SCHEMA_ENTRY
*/
CVEC utf_generate_schema(UTF_TABLE* utf,
                         SU_STRING* data_table,
                         SU_STRING* string_table,
                         const uint8_t utf_present);
                         

FU_FILE* utf_schema_to_fu(CVEC schema);

/*
    Returns a memory file with generated rows section.
*/
FU_FILE* utf_rows_to_fu(UTF_TABLE* utf, CVEC schema,
                        SU_STRING* string_table, SU_STRING* data_table,
                        const uint8_t utf_present);

/*
    For inserting data into schema.
    
    Returns 1 if data in all rows is the same.
    Otherwise 0.
*/
const uint8_t utf_column_rows_the_same(UTF_COLUMN* col);

/*

*/
void utf_table_row_to_record(UTF_ROW* row, UTF_RECORD* record, const uint8_t type,
                             SU_STRING* string_table, SU_STRING* data_table,
                             const uint8_t utf_present);

/*

*/                          
void utf_write_record_to_fu(FU_FILE* fu, UTF_RECORD* record, const uint8_t type);

/*
    
*/
const uint32_t utf_get_row_size(CVEC schema);

/*
    Returns true if the UTF table has any internal UTF tables
*/
const uint8_t utf_check_for_utf_tables(UTF_TABLE* utf);