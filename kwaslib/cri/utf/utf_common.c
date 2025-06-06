#include "utf_common.h"
#include "utf_defines.h"

#include <kwaslib/core/io/string_utils.h>

static const char* UTF_COLUMN_TYPE_STR[] =
{
    "UINT8", "SINT8", "UINT16", "SINT16",
    "UINT32", "SINT32", "UINT64", "SINT64",
    "FLOAT", "DOUBLE", "STRING", "VLDATA",
    "UINT128", "UNDEFINED"
};

uint8_t utf_get_type_size(const uint8_t type)
{
    switch(type)
    {
        case UTF_COLUMN_TYPE_UINT8:
        case UTF_COLUMN_TYPE_SINT8:
            return 1;   
        case UTF_COLUMN_TYPE_UINT16:
        case UTF_COLUMN_TYPE_SINT16:
            return 2;
        case UTF_COLUMN_TYPE_UINT32:
        case UTF_COLUMN_TYPE_SINT32:
        case UTF_COLUMN_TYPE_STRING:
        case UTF_COLUMN_TYPE_FLOAT:
            return 4;
        case UTF_COLUMN_TYPE_UINT64:
        case UTF_COLUMN_TYPE_SINT64:
        case UTF_COLUMN_TYPE_DOUBLE:
        case UTF_COLUMN_TYPE_VLDATA:
            return 8;
        case UTF_COLUMN_TYPE_UINT128:
            return 16;
    }
    
    return 0;
}

const char* utf_type_to_str(const uint8_t type)
{
    return UTF_COLUMN_TYPE_STR[type];
}

uint8_t utf_str_to_type(const char* str, const uint32_t size)
{
    uint8_t type = -1;
    
    if(su_cmp_char(str, size, "UINT8", 5) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_UINT8;
    if(su_cmp_char(str, size, "SINT8", 5) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_SINT8;
    if(su_cmp_char(str, size, "UINT16", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_UINT16;
    if(su_cmp_char(str, size, "SINT16", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_SINT16;
    if(su_cmp_char(str, size, "UINT32", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_UINT32;
    if(su_cmp_char(str, size, "SINT32", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_SINT32;
    if(su_cmp_char(str, size, "UINT64", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_UINT64;
    if(su_cmp_char(str, size, "SINT64", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_SINT64;
    if(su_cmp_char(str, size, "FLOAT", 5) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_FLOAT;
    if(su_cmp_char(str, size, "DOUBLE", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_DOUBLE;
    if(su_cmp_char(str, size, "STRING", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_STRING;
    if(su_cmp_char(str, size, "VLDATA", 6) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_VLDATA;
    if(su_cmp_char(str, size, "UINT128", 7) == SU_STRINGS_MATCH)
        type = UTF_COLUMN_TYPE_UINT128;
    
    return type;
}