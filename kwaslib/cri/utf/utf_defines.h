#pragma once

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

/* Column data types */
#define UTF_COLUMN_TYPE_UINT8       0x0
#define UTF_COLUMN_TYPE_SINT8       0x1
#define UTF_COLUMN_TYPE_UINT16      0x2
#define UTF_COLUMN_TYPE_SINT16      0x3
#define UTF_COLUMN_TYPE_UINT32      0x4
#define UTF_COLUMN_TYPE_SINT32      0x5
#define UTF_COLUMN_TYPE_UINT64      0x6
#define UTF_COLUMN_TYPE_SINT64      0x7
#define UTF_COLUMN_TYPE_FLOAT       0x8
#define UTF_COLUMN_TYPE_DOUBLE      0x9
#define UTF_COLUMN_TYPE_STRING      0xa /* u32 string offset */
#define UTF_COLUMN_TYPE_VLDATA      0xb /* u32 data offset + u32 data size */
#define UTF_COLUMN_TYPE_UINT128     0xc /* for GUIDs */
#define UTF_COLUMN_TYPE_UNDEFINED   0xf /* shouldn't exist */

#define UTF_MAGIC                   (const char*)"@UTF"
#define UTF_TABLE_HEADER_SIZE       0x18

#define UTF_DATA_BLOCK_SIZE     32
#define UTF_STRING_BLOCK_SIZE   32

/*
    Structures
*/
typedef union
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
    uint32_t str_offset;
    
    struct
    {
        uint32_t offset;
        uint32_t size;
    } vl;

    uint8_t u128[16];
} UTF_RECORD;

typedef struct
{
    uint8_t type: 4;        /* UTF_COLUMN_TYPE */
    uint8_t name : 1;       /* column has name in schema (may be empty) */
    uint8_t schema : 1;     /* data is in schema (const value for all rows) */
    uint8_t row: 1;         /* data is in row data */
    uint8_t all : 1;        /* shouldn't exist */
} UTF_SCHEMA_DESC;

typedef struct
{
    UTF_SCHEMA_DESC desc;
    uint32_t name_offset;
    UTF_RECORD record;
} UTF_SCHEMA_ENTRY;

typedef struct
{
    char id[4];
    uint32_t table_size;
} UTF_HEADER;

typedef struct
{
    uint16_t version;
    uint16_t rows_offset;
    uint32_t string_table_offset;
    uint32_t data_offset;
    uint32_t name_offset;    /* In string table */
    uint16_t columns_count;
    uint16_t rows_width;
    uint32_t rows_count;
} UTF_TABLE_HEADER;