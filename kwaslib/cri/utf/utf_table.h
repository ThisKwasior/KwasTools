#pragma once

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

#include <kwaslib/cri/acb/acb_command.h>
#include <kwaslib/cri/audio/awb.h>

#include "utf_defines.h"

#define UTF_TABLE_VL_NONE       0
#define UTF_TABLE_VL_UTF        1   /* UTF Table */
#define UTF_TABLE_VL_AFS2       2   /* AWB */
#define UTF_TABLE_VL_ACBCMD     3   /* ACB Command */

typedef struct UTF_TABLE UTF_TABLE;

typedef struct UTF_ROW
{
    union
    {
        uint8_t u8;
        int8_t s8;
        uint16_t u16;
        int16_t s16;
        uint32_t u32;
        int32_t s32;
        uint64_t u64;
        int64_t s64;
        float f32;
        double f64;
        SU_STRING* str;
        SU_STRING* vl;
        uint8_t u128[16];
    } data;
    
    /* Embedded data types */
    union
    {
        UTF_TABLE* utf;
        AWB_FILE* afs2;
        ACB_COMMAND acbcmd;
    } embed;
    
    uint8_t embed_type;
} UTF_ROW;

typedef struct UTF_COLUMN
{
    SU_STRING* name;
    uint8_t type;       /* UTF_COLUMN_TYPE */
    CVEC rows;          /* Array of UTF_ROW */
} UTF_COLUMN;

struct UTF_TABLE
{
    SU_STRING* name;
    CVEC columns;       /* Array of UTF_COLUMN */
};

/*
    Implementation
*/
UTF_TABLE* utf_table_create(const char* name);
UTF_TABLE* utf_table_destroy(UTF_TABLE* utf);

/*
    Creates a new table and preallocates columns and rows with default values.

    Returns a pointer to a new table.
*/
UTF_TABLE* utf_table_create_by_size(const char* name, const uint32_t columns, const uint32_t rows);

/*
    Adds a column at the end of column vector.
    Will also resize the rows vector to match first column.
    
    Returns a pointer to newly created column.
*/
UTF_COLUMN* utf_table_append_column(UTF_TABLE* utf, const char* name, const uint8_t type);

/*
    Appends a new row to all columns.
    
    Returns an id of new row.
*/
const uint32_t utf_table_append_row(UTF_TABLE* utf);

/*
    Removes a column by id and all of its data.
*/
void utf_table_remove_column_by_id(UTF_TABLE* utf, const uint32_t id);

/*
    Removes a row with specified id from all columns.
*/
void utf_table_remove_row_by_id(UTF_TABLE* utf, const uint32_t id);

/*
    Removes a row with specified id from specified column.
*/
void utf_table_remove_row_from_col_by_id(UTF_COLUMN* col, const uint32_t id);

/*
    Checks for data to remove from a row.
    String, VL and other structures with embedded data.
*/
void utf_table_free_row_data(UTF_ROW* row, const uint8_t column_type);

/*
    Gets a column by id from a table.

    Returns a pointer to a column.
*/
UTF_COLUMN* utf_table_get_column_by_id(UTF_TABLE* utf, const uint32_t id);

/*
    Gets a row by axis.
    
    Returns a pointer to row structure.
*/
UTF_ROW* utf_table_get_row_xy(UTF_TABLE* utf, const uint32_t x, const uint32_t y);

/*
    Gets a row from specified column by id.

    Returns a pointer to row structure.
*/
UTF_ROW* utf_table_get_row_from_col_by_id(UTF_COLUMN* col, const uint32_t id);

/*
    Checks for @UTF, AFS2 and ACB Commands in VLDATA.
    Will create proper structure, change embed_type and free respective VLDATA.
*/
void utf_check_for_embedded(UTF_TABLE* utf);

/*
    These functions will return respective column and row counts.
*/
const uint32_t utf_table_get_column_count(UTF_TABLE* utf);
const uint32_t utf_table_get_row_count(UTF_TABLE* utf);